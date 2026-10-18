// SPDX-License-Identifier: GPL-3.0-or-later
// Deliberately image-only AmigaDOS OFS creator and structural verifier.
// Never opens a block device or overwrites an existing file.
#define _POSIX_C_SOURCE 200809L
#include "ofs_primitives.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#error "O_NOFOLLOW is required for image safety"
#endif

enum {
    BLOCK = 512, LONGS = 128, RESERVED = 2, ROOT_BMAP_START = 79,
    ROOT_BMAP_COUNT = 25, BMAP_BITS = 127 * 32,
    EXT_POINTERS = 127, ROOT_EXT_POINTER = 104, ROOT_NAME = 432
};

static uint32_t be32(const unsigned char *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint32_t word(const unsigned char *p, unsigned int i)
{
    return be32(p + i * 4U);
}

static void put_word(unsigned char *p, unsigned int i, uint32_t v)
{
    p += i * 4U;
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >> 8);
    p[3] = (unsigned char)v;
}

static int block_io(int fd, uint32_t index, unsigned char *data, int writing)
{
    size_t done = 0;
    const off_t offset = (off_t)index * BLOCK;
    while (done < BLOCK) {
        ssize_t n;
        do {
            if (writing)
                n = pwrite(fd, data + done, BLOCK - done, offset + (off_t)done);
            else
                n = pread(fd, data + done, BLOCK - done, offset + (off_t)done);
        } while (n < 0 && errno == EINTR);
        if (n <= 0)
            return -1;
        done += (size_t)n;
    }
    return 0;
}

static int volume_geometry(int fd, uint32_t *total, uint32_t *bitmap_count)
{
    struct stat st;
    uint32_t bits;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
        st.st_size < (off_t)(1U << 20) ||
        st.st_size > (off_t)(1024ULL << 20) ||
        st.st_size % BLOCK != 0)
        return -1;
    *total = (uint32_t)((uint64_t)st.st_size / BLOCK);
    return ifs_ofs_bitmap_geometry(
        BLOCK, RESERVED, *total, &bits, bitmap_count) == 0 &&
        bits == BMAP_BITS ? 0 : -1;
}

static int verify(int fd)
{
    uint32_t blocks, bitmaps, root, extension, seen = 0;
    uint32_t *pointers = NULL;
    unsigned char *occupied = NULL;
    unsigned char boot[BLOCK], root_data[BLOCK], data[BLOCK];
    uint32_t i, j, extensions = 0;
    int result = -1;

    if (volume_geometry(fd, &blocks, &bitmaps) != 0)
        return -1;
    root = blocks / 2U;
    occupied = calloc(blocks, 1U);
    pointers = calloc(bitmaps, sizeof(*pointers));
    if (!occupied || !pointers)
        goto out;
    occupied[0] = occupied[1] = occupied[root] = 1U;

    if (block_io(fd, 0, boot, 0) != 0 ||
        memcmp(boot, "DOS\0", 4) != 0 ||
        block_io(fd, 1, data, 0) != 0 ||
        block_io(fd, root, root_data, 0) != 0 ||
        word(root_data, 0) != 2U || word(root_data, 3) != 72U ||
        word(root_data, 78) != UINT32_MAX ||
        word(root_data, 127) != 1U ||
        ifs_ofs_block_checksum(root_data, BLOCK) != 0U ||
        root_data[ROOT_NAME] > 30U ||
        ifs_ofs_validate_name(root_data + ROOT_NAME + 1U,
                              root_data[ROOT_NAME], 1) != IFS_OFS_NAME_OK)
        goto out;
    for (i = 0; i < BLOCK; ++i)
        if (data[i] != 0)
            goto out;

    for (i = 0; i < ROOT_BMAP_COUNT && seen < bitmaps; ++i)
        pointers[seen++] = word(root_data, ROOT_BMAP_START + i);
    extension = word(root_data, ROOT_EXT_POINTER);
    while (seen < bitmaps) {
        if (extension < RESERVED || extension >= blocks ||
            occupied[extension] || ++extensions > bitmaps ||
            block_io(fd, extension, data, 0) != 0)
            goto out;
        occupied[extension] = 1U;
        for (i = 0; i < EXT_POINTERS && seen < bitmaps; ++i)
            pointers[seen++] = word(data, i);
        extension = word(data, 127);
    }
    if (extension != 0U)
        goto out;
    for (i = 0; i < bitmaps; ++i) {
        if (pointers[i] < RESERVED || pointers[i] >= blocks ||
            occupied[pointers[i]])
            goto out;
        occupied[pointers[i]] = 1U;
    }
    for (i = 0; i < bitmaps; ++i) {
        if (block_io(fd, pointers[i], data, 0) != 0 ||
            ifs_ofs_block_checksum(data, BLOCK) != 0U)
            goto out;
        for (j = 0; j < BMAP_BITS; ++j) {
            const uint64_t number = (uint64_t)RESERVED +
                                    (uint64_t)i * BMAP_BITS + j;
            const unsigned int b = (unsigned int)j;
            const int is_free = (word(data, 1U + b / 32U) &
                                 (UINT32_C(1) << (b % 32U))) != 0U;
            if (is_free != (number < blocks &&
                            occupied[number] == 0U))
                goto out;
        }
    }
    result = 0;
out:
    free(pointers);
    free(occupied);
    return result;
}

static int create(int fd, const char *label)
{
    uint32_t blocks, bitmaps, root, extras, cursor = RESERVED;
    uint32_t *bmap = NULL, *ext = NULL;
    unsigned char *occupied = NULL;
    unsigned char data[BLOCK];
    uint32_t i, j;
    int result = -1;

    if (volume_geometry(fd, &blocks, &bitmaps) != 0 ||
        ifs_ofs_validate_name((const unsigned char *)label,
                              (uint32_t)strlen(label), 1) !=
                              IFS_OFS_NAME_OK || label[0] == '\0')
        return -1;
    root = blocks / 2U;
    extras = bitmaps > ROOT_BMAP_COUNT
        ? (bitmaps - ROOT_BMAP_COUNT + EXT_POINTERS - 1U) / EXT_POINTERS : 0U;
    bmap = calloc(bitmaps, sizeof(*bmap));
    ext = calloc(extras ? extras : 1U, sizeof(*ext));
    occupied = calloc(blocks, 1U);
    if (!bmap || !ext || !occupied)
        goto out;
    occupied[0] = occupied[1] = occupied[root] = 1U;
    for (i = 0; i < bitmaps + extras; ++i) {
        if (cursor == root)
            ++cursor;
        if (cursor >= blocks)
            goto out;
        if (i < bitmaps)
            bmap[i] = cursor;
        else
            ext[i - bitmaps] = cursor;
        occupied[cursor++] = 1U;
    }

    memset(data, 0, sizeof(data));
    memcpy(data, "DOS\0", 4);
    if (block_io(fd, 0, data, 1) != 0)
        goto out;
    memset(data, 0, sizeof(data));
    if (block_io(fd, 1, data, 1) != 0)
        goto out;

    for (i = 0; i < bitmaps; ++i) {
        memset(data, 0, sizeof(data));
        for (j = 0; j < BMAP_BITS; ++j) {
            const uint64_t number = (uint64_t)RESERVED +
                                    (uint64_t)i * BMAP_BITS + j;
            if (number < blocks && occupied[number] == 0U) {
                const unsigned int b = (unsigned int)j;
                const uint32_t existing = word(data, 1U + b / 32U);
                put_word(data, 1U + b / 32U,
                         existing | (UINT32_C(1) << (b % 32U)));
            }
        }
        put_word(data, 0, ifs_ofs_checksum_word_value(data, BLOCK, 0));
        if (block_io(fd, bmap[i], data, 1) != 0)
            goto out;
    }
    for (i = 0; i < extras; ++i) {
        const uint32_t start = ROOT_BMAP_COUNT + i * EXT_POINTERS;
        memset(data, 0, sizeof(data));
        for (j = 0; j < EXT_POINTERS && start + j < bitmaps; ++j)
            put_word(data, j, bmap[start + j]);
        put_word(data, 127, i + 1U < extras ? ext[i + 1U] : 0U);
        if (block_io(fd, ext[i], data, 1) != 0)
            goto out;
    }
    memset(data, 0, sizeof(data));
    put_word(data, 0, 2U);
    put_word(data, 3, 72U);
    put_word(data, 78, UINT32_MAX);
    for (i = 0; i < bitmaps && i < ROOT_BMAP_COUNT; ++i)
        put_word(data, ROOT_BMAP_START + i, bmap[i]);
    put_word(data, ROOT_EXT_POINTER, extras ? ext[0] : 0U);
    data[ROOT_NAME] = (unsigned char)strlen(label);
    memcpy(data + ROOT_NAME + 1U, label, strlen(label));
    put_word(data, 127, 1U);
    put_word(data, 5, ifs_ofs_checksum_word_value(data, BLOCK, 5));
    if (block_io(fd, root, data, 1) != 0 || fsync(fd) != 0 ||
        verify(fd) != 0)
        goto out;
    result = 0;
out:
    free(bmap);
    free(ext);
    free(occupied);
    return result;
}

static void usage(const char *name)
{
    fprintf(stderr,
            "Usage: %s --create FILE SIZE_MIB [LABEL]\n"
            "       %s --verify FILE\n"
            "Creates a new regular-file AmigaDOS OFS image (1..1024 MiB).\n"
            "Never overwrites an existing file or formats a block device.\n",
            name, name);
}

int main(int argc, char **argv)
{
    int fd, result;
    if (argc == 3 && strcmp(argv[1], "--verify") == 0) {
        struct stat st;
        fd = open(argv[2], O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        if (fd < 0)
            return perror("open image"), 1;
        if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
            fprintf(stderr, "Only regular-file images are supported.\n");
            close(fd);
            return 1;
        }
        result = verify(fd);
        close(fd);
        if (result != 0)
            return fprintf(stderr, "OFS image validation failed: %s\n", argv[2]), 1;
        printf("Validated AmigaDOS OFS image: %s\n", argv[2]);
        return 0;
    }
    if (argc >= 4 && argc <= 5 && strcmp(argv[1], "--create") == 0) {
        char *end = NULL;
        unsigned long mib;
        const char *label = argc == 5 ? argv[4] : "InfiltratorOFS";
        struct stat st;
        errno = 0;
        mib = strtoul(argv[3], &end, 10);
        if (errno || end == argv[3] || *end != '\0' ||
            mib < 1UL || mib > 1024UL ||
            ifs_ofs_validate_name((const unsigned char *)label,
                                  (uint32_t)strlen(label), 1) !=
                                  IFS_OFS_NAME_OK || label[0] == '\0') {
            usage(argv[0]);
            return 2;
        }
        fd = open(argv[2], O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW |
                            O_CLOEXEC, 0600);
        if (fd < 0)
            return perror("exclusive image creation"), 1;
        if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
            ftruncate(fd, (off_t)mib * 1024 * 1024) != 0) {
            fprintf(stderr, "Cannot create a regular-file image.\n");
            close(fd);
            unlink(argv[2]);
            return 1;
        }
        result = create(fd, label);
        if (close(fd) != 0)
            result = -1;
        if (result != 0) {
            unlink(argv[2]);
            fprintf(stderr, "OFS image creation or verification failed.\n");
            return 1;
        }
        printf("Created and verified AmigaDOS OFS image: %s (%lu MiB)\n",
               argv[2], mib);
        return 0;
    }
    usage(argv[0]);
    return 2;
}

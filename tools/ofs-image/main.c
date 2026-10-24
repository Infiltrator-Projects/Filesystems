// SPDX-License-Identifier: GPL-3.0-or-later
// AmigaDOS OFS creator and structural verifier.
// File creation never overwrites existing images.
// Linux partition formatting requires exclusive access and explicit confirmation.
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

#ifdef __linux__
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#endif

#ifndef O_NOFOLLOW
#error "O_NOFOLLOW is required for image safety"
#endif

/* OFS block-size geometry is independent of underlying 512-byte sectors.
 * Root metadata tail offsets are relative to the filesystem block's end. */
enum { MIN_BLOCK = 512, MAX_BLOCK = 32768, RESERVED = 2, ROOT_BMAP_COUNT = 25 };
static uint32_t block_size = MIN_BLOCK;
static uint32_t block_words(void) { return block_size / 4U; }
static uint32_t bitmap_bits(void) { return block_size * 8U - 32U; }
static uint32_t extension_pointers(void) { return block_words() - 1U; }
static uint32_t bitmap_pointer_start(void) { return block_words() - 49U; }
static uint32_t bitmap_extension_pointer(void) { return block_words() - 24U; }
static uint32_t disk_name_offset(void) { return block_size - 80U; }
static uint64_t max_image_bytes(void) { return (uint64_t)UINT32_MAX * block_size; }
static int select_block_size(const char *argument)
{
    char *end = NULL;
    unsigned long value;
    if (!argument || argument[0] == '-')
        return -1;
    errno = 0;
    value = strtoul(argument, &end, 10);
    if (errno || end == argument || *end != '\0' ||
        value < MIN_BLOCK || value > MAX_BLOCK ||
        (value & (value - 1U)) != 0U)
        return -1;
    block_size = (uint32_t)value;
    return 0;
}


/* One allocation bit per disk block, rather than one whole byte. */
static int marked(const unsigned char *bitmap, uint32_t number)
{
    return (bitmap[(size_t)number / 8U] &
            (unsigned char)(1U << (number % 8U))) != 0U;
}

static int mark(unsigned char *bitmap, uint32_t number)
{
    const size_t index = (size_t)number / 8U;
    const unsigned char mask = (unsigned char)(1U << (number % 8U));
    if ((bitmap[index] & mask) != 0U)
        return -1;
    bitmap[index] |= mask;
    return 0;
}

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
    const off_t offset = (off_t)((uint64_t)index * block_size);
    while (done < block_size) {
        ssize_t n;
        do {
            if (writing)
                n = pwrite(fd, data + done, block_size - done, offset + (off_t)done);
            else
                n = pread(fd, data + done, block_size - done, offset + (off_t)done);
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

    uint64_t bytes;
    if (fstat(fd, &st) != 0)
        return -1;
    if (S_ISREG(st.st_mode)) {
        if (st.st_size < 0)
            return -1;
        bytes = (uint64_t)st.st_size;
    }
#ifdef __linux__
    else if (S_ISBLK(st.st_mode)) {
        int sector_size;
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0 ||
            ioctl(fd, BLKSSZGET, &sector_size) != 0 ||
            sector_size != MIN_BLOCK)
            return -1;
    }
#endif
    else {
        return -1;
    }
    if (bytes < (uint64_t)(1U << 20) ||
        bytes > max_image_bytes() ||
        bytes % block_size != 0)
        return -1;
    *total = (uint32_t)(bytes / block_size);
    return ifs_ofs_bitmap_geometry(
        block_size, RESERVED, *total, &bits, bitmap_count) == 0 &&
        bits == bitmap_bits() ? 0 : -1;
}

static int verify_one_size(int fd)
{
    uint32_t blocks, bitmaps, root, extension, seen = 0;
    uint32_t *pointers = NULL;
    unsigned char *occupied = NULL;
    unsigned char boot[MAX_BLOCK], root_data[MAX_BLOCK], data[MAX_BLOCK];
    uint32_t i, j, extensions = 0;
    int result = -1;

    if (volume_geometry(fd, &blocks, &bitmaps) != 0)
        return -1;
    if (ifs_ofs_root_block(RESERVED, blocks, &root) != 0)
        goto out;
    occupied = calloc(((size_t)blocks + 7U) / 8U, 1U);
    pointers = calloc(bitmaps, sizeof(*pointers));
    if (!occupied || !pointers)
        goto out;
    if (mark(occupied, 0U) != 0 || mark(occupied, 1U) != 0 ||
        mark(occupied, root) != 0)
        goto out;

    if (block_io(fd, 0, boot, 0) != 0 ||
        memcmp(boot, "DOS\0", 4) != 0 ||
        block_io(fd, 1, data, 0) != 0 ||
        block_io(fd, root, root_data, 0) != 0 ||
        word(root_data, 0) != 2U || word(root_data, 3) != block_words() - 56U ||
        word(root_data, block_words() - 50U) != UINT32_MAX ||
        word(root_data, block_words() - 1U) != 1U ||
        ifs_ofs_block_checksum(root_data, block_size) != 0U ||
        root_data[disk_name_offset()] > 30U ||
        ifs_ofs_validate_name(root_data + disk_name_offset() + 1U,
                              root_data[disk_name_offset()], 1) != IFS_OFS_NAME_OK)
        goto out;
    for (i = 0; i < block_size; ++i)
        if (data[i] != 0)
            goto out;

    for (i = 0; i < ROOT_BMAP_COUNT && seen < bitmaps; ++i)
        pointers[seen++] = word(root_data, bitmap_pointer_start() + i);
    extension = word(root_data, bitmap_extension_pointer());
    while (seen < bitmaps) {
        if (extension < RESERVED || extension >= blocks ||
            marked(occupied, extension) || ++extensions > bitmaps ||
            block_io(fd, extension, data, 0) != 0)
            goto out;
        if (mark(occupied, extension) != 0)
            goto out;
        for (i = 0; i < extension_pointers() && seen < bitmaps; ++i)
            pointers[seen++] = word(data, i);
        extension = word(data, block_words() - 1U);
    }
    if (extension != 0U)
        goto out;
    for (i = 0; i < bitmaps; ++i) {
        if (pointers[i] < RESERVED || pointers[i] >= blocks ||
            marked(occupied, pointers[i]))
            goto out;
        if (mark(occupied, pointers[i]) != 0)
            goto out;
    }
    for (i = 0; i < bitmaps; ++i) {
        if (block_io(fd, pointers[i], data, 0) != 0 ||
            ifs_ofs_block_checksum(data, block_size) != 0U)
            goto out;
        for (j = 0; j < bitmap_bits(); ++j) {
            const uint64_t number = (uint64_t)RESERVED +
                                    (uint64_t)i * bitmap_bits() + j;
            const unsigned int b = (unsigned int)j;
            const int is_free = (word(data, 1U + b / 32U) &
                                 (UINT32_C(1) << (b % 32U))) != 0U;
            if (is_free != (number < blocks &&
                            !marked(occupied, (uint32_t)number)))
                goto out;
        }
    }
    result = 0;
out:
    free(pointers);
    free(occupied);
    return result;
}

/* DOS/0 has no independent block-size field; accept a candidate only
 * after validating its actual root, allocation bitmap and checksums. */
static int verify_any_size(int fd, int physical)
{
    uint32_t candidate;
    for (candidate = MIN_BLOCK; candidate <= MAX_BLOCK; candidate *= 2U) {
        if (physical && candidate != MIN_BLOCK)
            break;
        block_size = candidate;
        if (verify_one_size(fd) == 0)
            return 0;
    }
    return -1;
}

static int create(int fd, const char *label)
{
    uint32_t blocks, bitmaps, root, extras, cursor = RESERVED;
    uint32_t *bmap = NULL, *ext = NULL;
    unsigned char *occupied = NULL;
    unsigned char data[MAX_BLOCK];
    uint32_t i, j;
    int result = -1;

    if (volume_geometry(fd, &blocks, &bitmaps) != 0 ||
        ifs_ofs_validate_name((const unsigned char *)label,
                              (uint32_t)strlen(label), 1) !=
                              IFS_OFS_NAME_OK || label[0] == '\0')
        return -1;
    if (ifs_ofs_root_block(RESERVED, blocks, &root) != 0)
        goto out;
    extras = bitmaps > ROOT_BMAP_COUNT
        ? (bitmaps - ROOT_BMAP_COUNT + extension_pointers() - 1U) / extension_pointers() : 0U;
    bmap = calloc(bitmaps, sizeof(*bmap));
    ext = calloc(extras ? extras : 1U, sizeof(*ext));
    occupied = calloc(((size_t)blocks + 7U) / 8U, 1U);
    if (!bmap || !ext || !occupied)
        goto out;
    if (mark(occupied, 0U) != 0 || mark(occupied, 1U) != 0 ||
        mark(occupied, root) != 0)
        goto out;
    for (i = 0; i < bitmaps + extras; ++i) {
        if (cursor == root)
            ++cursor;
        if (cursor >= blocks)
            goto out;
        if (i < bitmaps)
            bmap[i] = cursor;
        else
            ext[i - bitmaps] = cursor;
        if (mark(occupied, cursor++) != 0)
            goto out;
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
        for (j = 0; j < bitmap_bits(); ++j) {
            const uint64_t number = (uint64_t)RESERVED +
                                    (uint64_t)i * bitmap_bits() + j;
            if (number < blocks && !marked(occupied, (uint32_t)number)) {
                const unsigned int b = (unsigned int)j;
                const uint32_t existing = word(data, 1U + b / 32U);
                put_word(data, 1U + b / 32U,
                         existing | (UINT32_C(1) << (b % 32U)));
            }
        }
        put_word(data, 0, ifs_ofs_checksum_word_value(data, block_size, 0));
        if (block_io(fd, bmap[i], data, 1) != 0)
            goto out;
    }
    for (i = 0; i < extras; ++i) {
        const uint32_t start = ROOT_BMAP_COUNT + i * extension_pointers();
        memset(data, 0, sizeof(data));
        for (j = 0; j < extension_pointers() && start + j < bitmaps; ++j)
            put_word(data, j, bmap[start + j]);
        put_word(data, block_words() - 1U, i + 1U < extras ? ext[i + 1U] : 0U);
        if (block_io(fd, ext[i], data, 1) != 0)
            goto out;
    }
    memset(data, 0, sizeof(data));
    put_word(data, 0, 2U);
    put_word(data, 3, block_words() - 56U);
    put_word(data, block_words() - 50U, UINT32_MAX);
    for (i = 0; i < bitmaps && i < ROOT_BMAP_COUNT; ++i)
        put_word(data, bitmap_pointer_start() + i, bmap[i]);
    put_word(data, bitmap_extension_pointer(), extras ? ext[0] : 0U);
    data[disk_name_offset()] = (unsigned char)strlen(label);
    memcpy(data + disk_name_offset() + 1U, label, strlen(label));
    put_word(data, block_words() - 1U, 1U);
    put_word(data, 5, ifs_ofs_checksum_word_value(data, block_size, 5));
    if (block_io(fd, root, data, 1) != 0 || fsync(fd) != 0 ||
        verify_one_size(fd) != 0)
        goto out;
    result = 0;
out:
    free(bmap);
    free(ext);
    free(occupied);
    return result;
}


#ifdef __linux__
/* Block-inode checks alone are insufficient: only a kernel-reported
 * partition (not the parent whole disk) is a valid destructive target.
 * Validate the identity on the already opened descriptor, not by filename.
 */
static int is_real_partition(int fd)
{
    struct stat st;
    char path[128], marker[32], *end;
    int handle, n;
    ssize_t len;
    unsigned long number;

    if (fstat(fd, &st) != 0 || !S_ISBLK(st.st_mode))
        return 0;
    n = snprintf(path, sizeof(path), "/sys/dev/block/%u:%u/partition",
                 major(st.st_rdev), minor(st.st_rdev));
    if (n < 1 || (size_t)n >= sizeof(path))
        return 0;
    handle = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (handle < 0)
        return 0;
    len = read(handle, marker, sizeof(marker) - 1U);
    (void)close(handle);
    if (len < 1)
        return 0;
    marker[len] = '\0';
    errno = 0;
    number = strtoul(marker, &end, 10);
    return errno == 0 && end != marker &&
           (*end == '\n' || *end == '\0') &&
           number >= 1UL && number <= UINT_MAX;
}

/* O_EXCL obtains the kernel's exclusive block-device claim and normally
 * fails with EBUSY if the volume is mounted or held by another writer.
 * No helper is allowed to auto-unmount or auto-repartition anything.
 */
static int operate_partition(const char *path, const char *label, int formatting)
{
    int fd, result;
    fd = open(path, (formatting ? O_RDWR : O_RDONLY) |
                    O_EXCL | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        perror("Cannot claim block partition exclusively; unmount it first");
        return 1;
    }
    if (!is_real_partition(fd)) {
        fprintf(stderr, "Refusing a whole disk or a non-partition device.\n");
        close(fd);
        return 1;
    }
    /* Keep physical-media writes at the previous qualified geometry.
     * Larger geometry must first pass independent EFI/Linux reader gates. */
    block_size = MIN_BLOCK;
    if (formatting) {
        uint64_t bytes = 0;
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0 ||
            bytes > (uint64_t)INT32_MAX * MIN_BLOCK) {
            fprintf(stderr, "Physical OFS above 1 TiB remains unqualified.\n");
            close(fd);
            return 1;
        }
    }
    result = formatting ? create(fd, label) : verify_any_size(fd, 1);
    if (close(fd) != 0)
        result = -1;
    if (result != 0) {
        fprintf(stderr, "OFS partition operation failed. "
                        "Do not mount an incompletely formatted volume.\n");
        return 1;
    }
    printf("%s AmigaDOS OFS partition: %s\n",
           formatting ? "Created and verified" : "Verified", path);
    return 0;
}
#endif

static void usage(const char *name)
{
    fprintf(stderr,
            "Usage: %s --create FILE SIZE_MIB [LABEL] [--block-size BYTES]\n"
            "       %s --create-blocks FILE TOTAL_BLOCKS [LABEL] [--block-size BYTES]\n"
            "       %s --verify FILE\n"
            "       %s --format-partition /dev/PART LABEL --confirm /dev/PART\n"
            "       %s --verify-partition /dev/PART\n"
            "Image blocks: 512, 1024, 2048, 4096, 8192, 16384, 32768 bytes.\n"
            "Partition mode: Linux only; 512-byte OFS blocks and\n"
            "unmounted, exclusive media; refuse above 1 TiB.\n"
            "The confirmation must repeat the exact partition path.\n",
            name, name, name, name, name);
}

int main(int argc, char **argv)
{
    int fd, result;
#ifdef __linux__
    if (argc == 6 && strcmp(argv[1], "--format-partition") == 0) {
        const char *label = argv[3];
        if (strcmp(argv[4], "--confirm") != 0 ||
            strcmp(argv[2], argv[5]) != 0 ||
            strncmp(argv[2], "/dev/", 5) != 0 ||
            label[0] == '\0' ||
            ifs_ofs_validate_name((const unsigned char *)label,
                                  (uint32_t)strlen(label), 1) !=
                                  IFS_OFS_NAME_OK) {
            fprintf(stderr, "Refusing: exact device confirmation or valid "
                            "volume label missing.\n");
            return 2;
        }
        return operate_partition(argv[2], label, 1);
    }
    if (argc == 3 && strcmp(argv[1], "--verify-partition") == 0) {
        if (strncmp(argv[2], "/dev/", 5) != 0)
            return 2;
        return operate_partition(argv[2], NULL, 0);
    }
#endif
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
        result = verify_any_size(fd, 0);
        close(fd);
        if (result != 0)
            return fprintf(stderr, "OFS image validation failed: %s\n", argv[2]), 1;
        printf("Validated AmigaDOS OFS image: %s\n", argv[2]);
        return 0;
    }
    if (argc >= 4 && argc <= 7 &&
        (strcmp(argv[1], "--create") == 0 ||
         strcmp(argv[1], "--create-blocks") == 0)) {
        const int blocks_mode = strcmp(argv[1], "--create-blocks") == 0;
        char *end = NULL;
        int positional = argc;
        unsigned long long units;
        uint64_t size_bytes;
        const char *label;
        if (argc >= 6 && strcmp(argv[argc - 2], "--block-size") == 0) {
            if (select_block_size(argv[argc - 1]) != 0) {
                usage(argv[0]);
                return 2;
            }
            positional -= 2;
        }
        if (positional != 4 && positional != 5) {
            usage(argv[0]);
            return 2;
        }
        label = positional == 5 ? argv[4] : "InfiltratorOFS";
        struct stat st;
        errno = 0;
        units = strtoull(argv[3], &end, 10);
        if (errno || end == argv[3] || *end != '\0' ||
            units < 1ULL ||
            (blocks_mode ?
                 (units > UINT32_MAX ||
                  units < (1024ULL * 1024ULL + block_size - 1U) / block_size) :
                 (units > max_image_bytes() / (1024ULL * 1024ULL))) ||
            ifs_ofs_validate_name((const unsigned char *)label,
                                  (uint32_t)strlen(label), 1) !=
                                  IFS_OFS_NAME_OK || label[0] == '\0') {
            usage(argv[0]);
            return 2;
        }
        size_bytes = blocks_mode ? (uint64_t)units * block_size :
                                   (uint64_t)units * 1024ULL * 1024ULL;
        fd = open(argv[2], O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW |
                            O_CLOEXEC, 0600);
        if (fd < 0)
            return perror("exclusive image creation"), 1;
        if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
            ftruncate(fd, (off_t)size_bytes) != 0) {
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
        printf("Created and verified AmigaDOS OFS image: %s (%llu %s)\n",
               argv[2], units, blocks_mode ? "blocks" : "MiB");
        return 0;
    }
    usage(argv[0]);
    return 2;
}

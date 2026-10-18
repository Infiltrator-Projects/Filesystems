/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "../native/filesystems/ofs/efi/reader.h"
#include "../native/filesystems/ofs/core/ofs_primitives.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Assertions must execute even in Release / -DNDEBUG builds. */
#define REQUIRE(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "REQUIRE failed at %s:%d: %s\\n", \
                __FILE__, __LINE__, #expr); \
        abort(); \
    } \
} while (0)

enum { COUNT = 1024, BLOCK = 512 };
static uint8_t media[COUNT][BLOCK];
static void put(uint8_t *b, unsigned word, uint32_t n)
{
    b[word * 4] = (uint8_t)(n >> 24);
    b[word * 4 + 1] = (uint8_t)(n >> 16);
    b[word * 4 + 2] = (uint8_t)(n >> 8);
    b[word * 4 + 3] = (uint8_t)n;
}
static void checksum(uint8_t *b)
{
    put(b, 5, ifs_ofs_checksum_word_value(b, BLOCK, 5));
}
static int rd(void *unused, uint32_t block, uint8_t out[512])
{
    (void)unused;
    if (block >= COUNT)
        return -1;
    memcpy(out, media[block], BLOCK);
    return 0;
}
static void label(uint8_t *b, const char *s)
{
    size_t n = strlen(s);
    REQUIRE(n <= 30U);
    b[432] = (uint8_t)n;
    memcpy(b + 433, s, n);
}
static void fixture(void)
{
    uint8_t *root = media[COUNT / 2], *dir = media[4];
    uint8_t *file = media[5], *data = media[6];
    unsigned slot;
    memset(media, 0, sizeof(media));
    put(media[0], 0, 0x444f5300U);
    put(root, 0, 2);
    put(root, 3, 72);
    put(root, 127, 1);
    label(root, "OFSBOOT");
    slot = ifs_ofs_directory_hash((const uint8_t *)"boot", 4, 72, 0);
    put(root, 6 + slot, 4);
    checksum(root);
    put(dir, 0, 2);
    put(dir, 1, 4);
    put(dir, 125, COUNT / 2);
    put(dir, 127, 2);
    label(dir, "boot");
    slot = ifs_ofs_directory_hash((const uint8_t *)"kernel.efi", 10, 72, 0);
    put(dir, 6 + slot, 5);
    checksum(dir);
    put(file, 0, 2);
    put(file, 1, 5);
    put(file, 2, 1);
    put(file, 4, 6);
    put(file, 81, 5);
    put(file, 125, 4);
    put(file, 127, (uint32_t)-3);
    label(file, "kernel.efi");
    checksum(file);
    put(data, 0, 8);
    put(data, 1, 5);
    put(data, 2, 1);
    put(data, 3, 5);
    memcpy(data + 24, "HELLO", 5);
    checksum(data);
}
int main(void)
{
    IfsOfsEfiVolume v = {0};
    IfsOfsEfiEntry directory, file, child;
    IfsOfsEfiDirCursor dcur = {0};
    IfsOfsEfiFileCursor fcur = {0};
    uint8_t name[31], buf[8] = {0};
    fixture();
    REQUIRE(ifs_ofs_efi_mount(&v, COUNT, rd, NULL) == 0);
    REQUIRE(ifs_ofs_efi_lookup(&v, &(IfsOfsEfiEntry){COUNT / 2, 0, IFS_OFS_EFI_DIRECTORY},
                              (const uint8_t *)"BOOT", 4, &directory) == 0);
    REQUIRE(ifs_ofs_efi_lookup(&v, &directory,
                              (const uint8_t *)"kernel.efi", 10, &file) == 0);
    REQUIRE(file.size == 5 && file.type == IFS_OFS_EFI_FILE);
    REQUIRE(ifs_ofs_efi_read(&v, &file, 0, buf, 5, &fcur) == 0);
    REQUIRE(memcmp(buf, "HELLO", 5) == 0);
    REQUIRE(ifs_ofs_efi_read(&v, &file, 2, buf, 3, &fcur) == 0);
    REQUIRE(memcmp(buf, "LLO", 3) == 0);
    REQUIRE(ifs_ofs_efi_read(&v, &file, 4, buf, 2, &fcur) == -1);
    REQUIRE(ifs_ofs_efi_next(&v, &directory, &dcur, &child, name) == 0);
    REQUIRE(strcmp((const char *)name, "kernel.efi") == 0);
    REQUIRE(ifs_ofs_efi_next(&v, &directory, &dcur, &child, name) == 1);
    REQUIRE(ifs_ofs_efi_lookup(&v, &directory,
                              (const uint8_t *)"missing", 7, &child) == 1);
    media[6][25] ^= 1;
    REQUIRE(ifs_ofs_efi_read(&v, &file, 0, buf, 5, NULL) == -1);
    fixture();
    put(media[5], 4, COUNT + 100U);
    checksum(media[5]);
    REQUIRE(ifs_ofs_efi_read(&v, &file, 0, buf, 5, NULL) == -1);
    fixture();
    put(media[0], 0, 0x444f5301U); /* FFS must not bind as OFS. */
    REQUIRE(ifs_ofs_efi_mount(&v, COUNT, rd, NULL) == -1);
    puts("OFS freestanding reader tests passed");
    return 0;
}

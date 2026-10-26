/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Independent in-memory FAT16 fixture. No host filesystem or block devices. */
#include "fat16_module.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SECTOR 512u
#define SECTORS 16384u
#define FAT_SECTORS 32u
#define ROOT_SECTORS 32u
#define DATA_START (1u + 2u * FAT_SECTORS + ROOT_SECTORS)
#define CLUSTER_SECTORS 2u
static unsigned char image[SECTORS * SECTOR];

static void p16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
}
static void p32(unsigned char *p, uint32_t v)
{
    p16(p, (unsigned)(v & 0xffffu));
    p16(p + 2, (unsigned)(v >> 16));
}
static int read_at(void *ctx, uint64_t offset, void *buf, uint32_t bytes)
{
    (void)ctx;
    if (offset > sizeof image || bytes > sizeof image - offset) return -1;
    memcpy(buf, image + (size_t)offset, bytes);
    return 0;
}
static void fat(unsigned cluster, unsigned next)
{
    p16(image + SECTOR + cluster * 2u, next);
    p16(image + (1u + FAT_SECTORS) * SECTOR + cluster * 2u, next);
}
static void fixture(void)
{
    memset(image, 0, sizeof image);
    unsigned char *b = image;
    b[0] = 0xebu;
    b[2] = 0x90u;
    p16(b + 11, SECTOR);
    b[13] = CLUSTER_SECTORS;
    p16(b + 14, 1);
    b[16] = 2;
    p16(b + 17, ROOT_SECTORS * 16u);
    p16(b + 19, SECTORS);
    b[21] = 0xf8u;
    p16(b + 22, FAT_SECTORS);
    memcpy(b + 54, "FAT16   ", 8);
    b[510] = 0x55u;
    b[511] = 0xaau;
    fat(0, 0xfff8u);
    fat(1, 0xffffu);
    fat(2, 5u);
    fat(5, 3u);
    fat(3, 0xffffu);

    unsigned char *dir = image + (1u + 2u * FAT_SECTORS) * SECTOR;
    memcpy(dir, "HELLO   TXT", 11);
    dir[11] = 0x20u;
    p16(dir + 26, 2u);
    p32(dir + 28, 3000u);
    const unsigned clusters[] = {2u,5u,3u};
    for (unsigned i=0; i<3; ++i) {
        memset(image + (DATA_START + (clusters[i]-2u)*CLUSTER_SECTORS)*SECTOR,
               'A'+(int)i, CLUSTER_SECTORS*SECTOR);
    }
}

static int failure(const char *msg);
/* Formatter/fixture patterns below are ported from Defragmenter Test Media's
 * test_media_raw_fat.c. They exercise nested directories and VFAT rather
 * than mistaking ABI-symbol presence for functioning namespace support. */
static uint8_t checksum(const uint8_t *entry)
{
    uint8_t sum = 0;
    for (unsigned i = 0; i < 11; ++i)
        sum = (uint8_t)(((sum & 1u) ? 0x80u : 0u) + (sum >> 1) + entry[i]);
    return sum;
}
static void long_slot(uint8_t *lfn, const char *name, const uint8_t *alias)
{
    static const uint8_t positions[13] =
        {1,3,5,7,9,14,16,18,20,22,24,28,30};
    size_t len = strlen(name);
    memset(lfn, 0xff, 32);
    lfn[0] = 0x41;
    lfn[11] = 0x0f;
    lfn[12] = 0;
    lfn[13] = checksum(alias);
    lfn[26] = lfn[27] = 0;
    for (unsigned i = 0; i < 13; ++i)
        p16(lfn + positions[i], i < len ? (uint8_t)name[i] :
            i == len ? 0u : 0xffffu);
}
static int namespace_tests(void)
{
    const IfsFat16Module *v1 = infiltrator_filesystem_module_v1();
    const IfsFat16ModuleV2 *v2 = infiltrator_filesystem_module_v2();
    IfsFat16Io io = {read_at, NULL, sizeof image};
    IfsFat16Volume volume;
    IfsFat16Dirent entry;
    uint8_t out[6] = {0};
    uint8_t *root, *subdir;
    if (!v2 || v2->abi_version != IFS_FAT16_ABI_V2 ||
        v2->structure_bytes != sizeof(*v2) ||
        strcmp(v2->format_id, "fat16") != 0 ||
        !(v2->capabilities & IFS_FAT16_CAP_DIRECTORY) ||
        !(v2->capabilities & IFS_FAT16_CAP_VFAT_LFN))
        return failure("ABI v2 directory discovery");
    fixture();
    if (v1->probe(&io, &volume)) return failure("namespace fixture probe");
    root = image + (1u + 2u * FAT_SECTORS) * SECTOR;
    /* Root entries: HELLO.TXT, long-named file, SUBDIR. */
    uint8_t *alias = root + 2 * 32u;
    memcpy(alias, "LONGFI~1TXT", 11);
    alias[11] = 0x20;
    p16(alias + 26, 7);
    p32(alias + 28, 5);
    long_slot(root + 32u, "Long.txt", alias);
    uint8_t *dir = root + 3 * 32u;
    memcpy(dir, "SUBDIR     ", 11);
    dir[11] = 0x10;
    p16(dir + 26, 6);
    fat(6, 0xffffu);
    fat(7, 0xffffu);
    fat(8, 0xffffu);
    memset(image + (DATA_START + 5u * CLUSTER_SECTORS) * SECTOR, 'N', 5);
    subdir = image + (DATA_START + 4u * CLUSTER_SECTORS) * SECTOR;
    memcpy(subdir, "CHILD   TXT", 11);
    subdir[11] = 0x20u;
    p16(subdir + 26, 8);
    p32(subdir + 28, 5);
    memset(image + (DATA_START + 6u * CLUSTER_SECTORS) * SECTOR, 'C', 5);

    if (v2->list_directory(&io, &volume, 0, 0, &entry) != 0 ||
        strcmp(entry.name, "HELLO.TXT") != 0)
        return failure("root short-name enumeration");
    if (v2->list_directory(&io, &volume, 0, 1, &entry) != 0 ||
        strcmp(entry.name, "Long.txt") != 0 ||
        strcmp(entry.short_name, "LONGFI~1.TXT") != 0)
        return failure("VFAT long-name enumeration");
    if (v2->read_entry(&io, &volume, &entry, 0, out, 5) != 0 ||
        memcmp(out, "NNNNN", 5))
        return failure("read long-name file contents");
    if (v2->list_directory(&io, &volume, 0, 2, &entry) != 0 ||
        strcmp(entry.name, "SUBDIR") != 0 ||
        (entry.attributes & 0x10u) == 0)
        return failure("root subdirectory discovery");
    if (v2->read_entry(&io, &volume, &entry, 0, out, 5) == 0)
        return failure("directory file-read was accepted");
    if (v2->list_directory(&io, &volume, entry.first_cluster, 0, &entry) != 0 ||
        strcmp(entry.name, "CHILD.TXT") != 0)
        return failure("nested directory traversal");
    if (v2->read_entry(&io, &volume, &entry, 0, out, 5) != 0 ||
        memcmp(out, "CCCCC", 5))
        return failure("read nested file");
    if (v2->list_directory(&io, &volume, 6, 1, &entry) != 1)
        return failure("nested directory end");
    /* Bad VFAT checksum must fall back to the validated short name. */
    root[32u + 13u] ^= 1u;
    if (v2->list_directory(&io, &volume, 0, 1, &entry) != 0 ||
        strcmp(entry.name, "LONGFI~1.TXT") != 0)
        return failure("corrupt VFAT checksum accepted");
    /* A directory with no end marker and a cyclic chain must fail closed. */
    memset(subdir, 0xe5u, CLUSTER_SECTORS * SECTOR);
    fat(6, 6u);
    if (v2->list_directory(&io, &volume, 6, 0, &entry) != -1)
        return failure("directory FAT cycle not rejected");
    /* A corrupt chain marker must fail rather than wrap to another cluster. */
    fat(6, 0xfff7u);
    if (v2->list_directory(&io, &volume, 6, 0, &entry) != -1)
        return failure("bad FAT cluster not rejected");
    return 0;
}

static int failure(const char *msg)
{
    fprintf(stderr, "FAT16 test failure: %s\n", msg);
    return 1;
}
int main(void)
{
    const IfsFat16Module *api = infiltrator_filesystem_module_v1();
    IfsFat16Io io = {read_at, NULL, sizeof image};
    IfsFat16Volume vol;
    IfsFat16File file;
    uint32_t fragments;
    unsigned char data[3000];

    if (!api || api->abi_version != 1 || api->structure_bytes != sizeof(*api) ||
        strcmp(api->format_id, "fat16") != 0 ||
        (api->capabilities & IFS_FAT16_CAP_FILE_READ) == 0 ||
        (api->capabilities & IFS_FAT16_CAP_ROOT_FRAGMENT_COUNT) == 0)
        return failure("bad module ABI or capabilities");
    fixture();
    if (api->probe(&io, &vol) != 0 || vol.cluster_count < 4085 ||
        vol.cluster_count >= 65525 || vol.cluster_bytes != 1024)
        return failure("valid FAT16 image not detected");
    if (api->root_file(&io, &vol, 0, &file) != 0 ||
        strcmp(file.name, "HELLO.TXT") != 0 || file.size_bytes != 3000)
        return failure("root directory missing");
    if (api->root_file(&io, &vol, 1, &file) != 1)
        return failure("extra root entry");
    if (api->root_file(&io, &vol, 0, &file) != 0 ||
        api->root_file_fragments(&io, &vol, &file, &fragments) != 0 ||
        fragments != 3u)
        return failure("fragment count wrong");
    if (api->read_file(&io, &vol, &file, 0, data, sizeof data) != 0)
        return failure("read failed");
    for (unsigned i=0; i<3000; ++i)
        if (data[i] != (unsigned char)('A' + (int)(i/1024u)))
            return failure("noncontiguous chain was read incorrectly");
    if (api->read_file(&io, &vol, &file, 2999u, data, 2u) == 0)
        return failure("out-of-bounds file read accepted");

    fat(5, 0xffffu);
    if (api->root_file_fragments(&io, &vol, &file, &fragments) == 0)
        return failure("short/corrupt chain accepted");
    fixture();
    image[13] = 8u; /* lower cluster count => FAT12, despite FAT16 text */
    if (api->probe(&io, &vol) == 0)
        return failure("FAT12 accepted as FAT16");
    fixture();
    image[510] = 0;
    if (api->probe(&io, &vol) == 0)
        return failure("broken signature accepted");
    fixture();
    io.device_bytes = 512;
    if (api->probe(&io, &vol) == 0)
        return failure("truncated image accepted");
    if (namespace_tests() != 0) return 1;
    puts("FAT16-MODULE-READ-DIRECTORY-LFN-PASS");
    return 0;
}

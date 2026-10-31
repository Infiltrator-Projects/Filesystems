/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Independent 35 MiB FAT32 image: packed on-media FAT32 structures only. */
#include "fat32_module.h"
#include <stdio.h>
#include <string.h>
#define SECTOR 512u
#define SECTORS 70000u
#define RESERVED 32u
#define FAT_SECTORS 600u
#define DATA_START (RESERVED + 2u * FAT_SECTORS)
static unsigned char image[SECTORS * SECTOR];
static void p16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)v; p[1] = (unsigned char)(v >> 8);
}
static void p32(unsigned char *p, uint32_t v)
{
    p16(p, v & 0xffffu); p16(p+2, v >> 16);
}
static int read_at(void *opaque, uint64_t at, void *out, uint32_t n)
{
    (void)opaque;
    if (at > sizeof image || n > sizeof image - at) return -1;
    memcpy(out, image + (size_t)at, n);
    return 0;
}
static unsigned recorded_writes, recorded_flushes;
static int write_at(void *opaque, uint64_t at,
                    const void *source, uint32_t count)
{
    (void)opaque;
    if (at < (uint64_t)DATA_START * SECTOR ||
        at > sizeof image || count > sizeof image - at)
        return -1; /* Never mutate BPB, FAT copies or root metadata. */
    memcpy(image + (size_t)at, source, count);
    ++recorded_writes;
    return 0;
}
static int flush(void *opaque)
{
    (void)opaque;
    ++recorded_flushes;
    return 0;
}
static unsigned char *data(unsigned cluster)
{
    return image + (DATA_START + (cluster - 2u)) * SECTOR;
}
static void fat(unsigned cluster, uint32_t value)
{
    for (unsigned copy = 0; copy < 2u; ++copy)
        p32(image + (RESERVED + copy * FAT_SECTORS) * SECTOR + cluster * 4u,
            value);
}
static void fixture(void)
{
    memset(image, 0, sizeof image);
    unsigned char *b = image;
    b[0] = 0xeb; b[2] = 0x90;
    p16(b + 11, SECTOR);
    b[13] = 1u;
    p16(b + 14, RESERVED);
    b[16] = 2u;
    p16(b + 17, 0u);
    p16(b + 19, 0u);
    b[21] = 0xf8u;
    p16(b + 22, 0u);
    p32(b + 32, SECTORS);
    p32(b + 36, FAT_SECTORS);
    p32(b + 44, 2u);
    memcpy(b + 82, "FAT16   ", 8); /* Format label cannot change FAT32 semantics. */
    b[510] = 0x55u; b[511] = 0xaau;
    fat(0u, UINT32_C(0x0ffffff8));
    fat(1u, UINT32_C(0x0fffffff));
    fat(2u, 6u); /* Root directory traverses a real cluster chain. */
    fat(6u, UINT32_C(0x0fffffff));
    fat(3u, 7u); fat(7u, 4u); fat(4u, UINT32_C(0x0fffffff));
    memset(data(2u), 0xe5, SECTOR); /* All 16 entries are deleted. */
    unsigned char *dir = data(6u);
    memcpy(dir, "HELLO   TXT", 11);
    dir[11] = 0x20u;
    p16(dir + 20, 0u); p16(dir + 26, 3u);
    p32(dir + 28, 1200u);
    const unsigned chain[] = {3u, 7u, 4u};
    for (unsigned i = 0; i < 3u; ++i)
        memset(data(chain[i]), (int)('A' + i), SECTOR);
}
static int fail(const char *reason)
{
    fprintf(stderr, "fat32_module_test: %s\n", reason);
    return 1;
}
int main(void)
{
    const IfsFat32Module *api = infiltrator_filesystem_module_v1();
    const IfsFat32ModuleV2 *v2 = infiltrator_filesystem_module_v2();
    IfsFat32WritableIo writable = {{read_at, NULL, sizeof image}, write_at, flush};
    IfsFat32Io io = {read_at, NULL, sizeof image};
    IfsFat32Volume vol = {0};
    IfsFat32File file = {0};
    uint32_t fragments;
    unsigned char bytes[1200];
    if (!api || api->abi_version != 1u ||
        api->structure_bytes != sizeof(*api) ||
        strcmp(api->format_id, "fat32") != 0 ||
        (api->capabilities & IFS_FAT32_CAP_FILE_READ) == 0 ||
        (api->capabilities & IFS_FAT32_CAP_ROOT_FRAGMENT_COUNT) == 0)
        return fail("ABI/identity missing");
    fixture();
    if (api->probe(&io, &vol) != 0 || vol.root_cluster != 2u ||
        vol.cluster_count < 65525u || vol.cluster_bytes != SECTOR)
        return fail("real FAT32 geometry rejected");
    if (api->root_file(&io, &vol, 0u, &file) != 0 ||
        strcmp(file.name, "HELLO.TXT") != 0 || file.size_bytes != 1200u)
        return fail("root directory cluster traversal failed");
    if (api->root_file(&io, &vol, 1u, &file) != 1)
        return fail("extra root directory entry accepted");
    if (api->root_file_fragments(&io, &vol, &file, &fragments) != 0 ||
        fragments != 3u)
        return fail("fragment count incorrect");
    if (api->read_file(&io, &vol, &file, 0u, bytes, sizeof bytes) != 0)
        return fail("fragmented file read failed");
    for (unsigned i = 0; i < sizeof bytes; ++i)
        if (bytes[i] != (unsigned char)('A' + i / SECTOR))
            return fail("FAT32 file contents incorrect");
    if (api->read_file(&io, &vol, &file, 1199u, bytes, 2u) == 0)
        return fail("out-of-file read accepted");

    if (!v2 || v2->abi_version != IFS_FAT32_V2_ABI_VERSION ||
        v2->structure_bytes != sizeof(*v2) ||
        strcmp(v2->format_id, "fat32") != 0 ||
        v2->readonly_v1 != api ||
        (v2->capabilities & IFS_FAT32_V2_CAP_OVERWRITE_EXISTING) == 0u)
        return fail("V2 FAT32 ABI/identity mismatch");
    if (v2->overwrite_existing(&writable, &vol, &file,
            510u, "WXYZ", 4u) != 0 ||
        recorded_writes != 2u || recorded_flushes != 1u ||
        api->read_file(&io, &vol, &file, 510u, bytes, 4u) != 0 ||
        memcmp(bytes, "WXYZ", 4u) != 0)
        return fail("V2 fragmented cross-cluster overwrite failed");
    if (v2->overwrite_existing(&writable, &vol, &file,
            1199u, "XX", 2u) == 0 || recorded_writes != 2u)
        return fail("V2 allowed file extension");
    {
        IfsFat32File forged = file;
        forged.first_cluster = 9u;
        if (v2->overwrite_existing(&writable, &vol, &forged,
               0u, "X", 1u) == 0 || recorded_writes != 2u)
            return fail("V2 allowed forged root metadata");
        forged = file;
        forged.attributes |= 1u;
        if (v2->overwrite_existing(&writable, &vol, &forged,
               0u, "X", 1u) == 0 || recorded_writes != 2u)
            return fail("V2 ignored read-only file attribute");
    }
    writable.flush = NULL;
    if (v2->overwrite_existing(&writable, &vol, &file,
            0u, "X", 1u) == 0 || recorded_writes != 2u)
        return fail("V2 allowed writer without flush");
    writable.flush = flush;
    writable.write_at = NULL;
    if (v2->overwrite_existing(&writable, &vol, &file,
            0u, "X", 1u) == 0 || recorded_writes != 2u)
        return fail("V2 allowed absent writer");
    writable.write_at = write_at;
    {
        IfsFat32Volume tampered = vol;
        ++tampered.data_offset;
        if (v2->overwrite_existing(&writable, &tampered, &file,
                0u, "X", 1u) == 0 || recorded_writes != 2u)
            return fail("V2 accepted altered volume geometry");
    }
    p16(image + 40u, 0x0081u);
    if (v2->overwrite_existing(&writable, &vol, &file,
            0u, "X", 1u) == 0 || recorded_writes != 2u)
        return fail("V2 accepted unqualified FAT32 active FAT selection");
    p16(image + 40u, 0u);
    fat(7u, UINT32_C(0x0fffffff));
    if (v2->overwrite_existing(&writable, &vol, &file,
            0u, "X", 1u) == 0 || recorded_writes != 2u)
        return fail("V2 wrote through corrupt FAT32 allocation chain");
    if (api->root_file_fragments(&io, &vol, &file, &fragments) == 0)
        return fail("short FAT32 file chain accepted");
    fixture();
    fat(2u, 0u);
    if (api->root_file(&io, &vol, 0u, &file) == 0)
        return fail("invalid root FAT chain accepted");
    fixture();
    image[13] = 2u; /* FAT16 cluster range, regardless of FAT32 label. */
    if (api->probe(&io, &vol) == 0)
        return fail("FAT16 sized partition falsely probed as FAT32");
    fixture();
    p32(image + 44, 1u); /* Reserved cluster may not be root. */
    if (api->probe(&io, &vol) == 0)
        return fail("invalid root cluster accepted");
    fixture();
    image[RESERVED * SECTOR] = 0u;
    if (api->probe(&io, &vol) == 0)
        return fail("invalid FAT32 reserved entry accepted");
    fixture();
    io.device_bytes = 512u;
    if (api->probe(&io, &vol) == 0)
        return fail("truncated partition accepted");
    puts("FAT32-MODULE-READ-ROOT-CLUSTER-AND-FRAGMENT-PASS");
    return 0;
}

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
static unsigned recorded_writes, recorded_flushes;
static int write_at(void *ctx, uint64_t offset,
                    const void *buf, uint32_t bytes)
{
    (void)ctx;
    if (offset > sizeof image || bytes > sizeof image - offset)
        return -1;
    memcpy(image + (size_t)offset, buf, bytes);
    ++recorded_writes;
    return 0;
}
static int flush(void *ctx)
{
    (void)ctx;
    ++recorded_flushes;
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
static int failure(const char *msg)
{
    fprintf(stderr, "FAT16 test failure: %s\n", msg);
    return 1;
}
int main(void)
{
    const IfsFat16Module *api = infiltrator_filesystem_module_v1();
    const IfsClassicFatModuleV2 *v2 = infiltrator_filesystem_module_v2();
    IfsClassicFatWritableIo writable = {{read_at, NULL, sizeof image}, write_at, flush};
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


    /* V1 stays unchanged. V2 writes only already allocated regular-file data. */
    if (!v2 || v2->abi_version != IFS_CLASSIC_FAT_V2_ABI_VERSION ||
        v2->structure_bytes != sizeof(*v2) ||
        strcmp(v2->format_id, "fat16") != 0 ||
        v2->readonly_v1 != api ||
        (v2->capabilities & IFS_CLASSIC_FAT_V2_CAP_OVERWRITE_EXISTING) == 0u)
        return failure("V2 ABI incorrectly advertised");
    if (v2->overwrite_existing(&writable, &vol, &file,
                1022u, "WXYZ", 4u) != 0 ||
        recorded_writes != 2u || recorded_flushes != 1u ||
        api->read_file(&io, &vol, &file, 1022u, data, 4u) != 0 ||
        memcmp(data, "WXYZ", 4u) != 0)
        return failure("V2 cross-cluster overwrite or flush failed");
    if (v2->overwrite_existing(&writable, &vol, &file,
            2999u, "XX", 2u) == 0 ||
        recorded_writes != 2u)
        return failure("V2 allowed extension past file size");
    {
        IfsClassicFatFile forged = file;
        forged.first_cluster = 4u;
        if (v2->overwrite_existing(&writable, &vol, &forged,
                0u, "X", 1u) == 0 || recorded_writes != 2u)
            return failure("V2 accepted forged file metadata");
        forged = file;
        forged.attributes |= 0x01u;
        if (v2->overwrite_existing(&writable, &vol, &forged,
                0u, "X", 1u) == 0 || recorded_writes != 2u)
            return failure("V2 ignored read-only attribute");
    }
    writable.flush = NULL;
    if (v2->overwrite_existing(&writable, &vol, &file,
            0u, "X", 1u) == 0 || recorded_writes != 2u)
        return failure("V2 permitted write without flush");
    writable.flush = flush;
    writable.write_at = NULL;
    if (v2->overwrite_existing(&writable, &vol, &file,
            0u, "X", 1u) == 0 || recorded_writes != 2u)
        return failure("V2 permitted write without host writer");
    writable.write_at = write_at;
    {
        IfsClassicFatVolume forged_volume = vol;
        forged_volume.data_offset += SECTOR;
        if (v2->overwrite_existing(&writable, &forged_volume, &file,
                0u, "X", 1u) == 0 || recorded_writes != 2u)
            return failure("V2 accepted stale/forged volume geometry");
    }

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
    puts("FAT16-MODULE-READ-AND-FRAGMENT-PASS");
    return 0;
}

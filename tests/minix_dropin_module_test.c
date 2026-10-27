/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent synthetic Minix V1/V2/V3 media, bounds and indirect-tree tests. */
#include "minix_module.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define BS 1024u
#define ZONES 64u
static uint8_t disk[ZONES * BS];
static unsigned reads;
static void w16(uint8_t *p, uint32_t x) { p[0] = (uint8_t)x; p[1] = (uint8_t)(x>>8); }
static void w32(uint8_t *p, uint32_t x) {
    p[0] = (uint8_t)x; p[1] = (uint8_t)(x>>8);
    p[2] = (uint8_t)(x>>16); p[3] = (uint8_t)(x>>24);
}
static int io_read(void *u, uint64_t off, void *dst, uint32_t n)
{
    (void)u;
    if (off > sizeof disk || n > sizeof disk - off) return -1;
    memcpy(dst, disk + (size_t)off, n);
    ++reads;
    return 0;
}
static int fail(const char *why) { fprintf(stderr, "MINIX: %s\n", why); return 1; }
static void inode_init(uint8_t *p, uint32_t version, uint16_t mode,
                       uint32_t bytes, uint32_t first)
{
    w16(p, mode);
    if (version == 1u) {
        w32(p + 4, bytes);
        p[13] = 1u;
        w16(p + 14, first);
    } else {
        w16(p + 2, 1u);
        w32(p + 8, bytes);
        w32(p + 24, first);
    }
}
static void inode_zone(uint8_t *inode, uint32_t version,
                       uint32_t slot, uint32_t zone)
{
    if (version == 1u) w16(inode + 14u + slot*2u, zone);
    else w32(inode + 24u + slot*4u, zone);
}
static void put_directory(uint8_t *p, uint32_t version, uint32_t stride,
                          uint32_t slot, uint32_t ino, const char *name)
{
    p += slot * stride;
    if (version == 3u) { w32(p, ino); p += 4; }
    else { w16(p, ino); p += 2; }
    while (*name) *p++ = (uint8_t)*name++;
}
static void fixture(uint32_t version, uint32_t namelen)
{
    uint8_t *sb, *root, *file, *dir;
    uint32_t stride = version == 3u ? 64u : namelen + 2u;
    uint32_t inode_size = version == 1u ? 32u : 64u;
    memset(disk, 0, sizeof disk);
    sb = disk + BS;
    if (version == 3u) {
        w32(sb, 64u);                  /* inode count */
        w16(sb + 6u, 1u);             /* inode bitmap blocks */
        w16(sb + 8u, 1u);             /* zone bitmap blocks */
        w16(sb + 10u, 8u);            /* first data zone */
        w32(sb + 16u, 100000u);       /* maximum file bytes */
        w32(sb + 20u, ZONES);
        w16(sb + 24u, 0x4d5au);
        w16(sb + 28u, BS);
    } else {
        w16(sb, 64u);
        w16(sb + 2u, ZONES);
        w16(sb + 4u, 1u);
        w16(sb + 6u, 1u);
        w16(sb + 8u, 8u);
        w32(sb + 12u, 100000u);
        w16(sb + 16u, version == 1u ?
            (namelen == 14u ? 0x137fu : 0x138fu) :
            (namelen == 14u ? 0x2468u : 0x2478u));
        if (version == 2u) w32(sb + 20u, ZONES);
    }
    disk[2u * BS] = 7u;               /* reserved, root and file inodes */
    disk[3u * BS] = 0xffu;
    root = disk + 4u*BS;
    file = root + inode_size;
    inode_init(root, version, 0x41edu, 3u * stride, 8u);
    inode_init(file, version, 0x81a4u, 9u*BS+5u, 10u);
    for (uint32_t i = 0u; i < 7u; ++i)
        inode_zone(file, version, i, 10u + i);
    inode_zone(file, version, 7u, 24u);   /* single indirect */
    uint8_t *ind = disk + 24u*BS;
    for (uint32_t i = 0u; i < 3u; ++i) {
        if (version == 1u) w16(ind + 2u*i, 20u + i);
        else w32(ind + 4u*i, 20u + i);
    }
    for (uint32_t i = 0u; i < 10u; ++i) {
        uint32_t zone = i < 7u ? 10u + i : 20u + i - 7u;
        memset(disk + zone*BS, 'A'+(int)i, BS);
    }
    dir = disk + 8u*BS;
    put_directory(dir, version, stride, 0u, 1u, ".");
    put_directory(dir, version, stride, 1u, 1u, "..");
    put_directory(dir, version, stride, 2u, 2u, "HELLO.TXT");
}
static int run_variant(uint32_t version, uint32_t name_bytes)
{
    const IfsMinixModule *api = infiltrator_filesystem_module_v1();
    IfsMinixVolume volume;
    IfsMinixInode root, file, forged;
    IfsMinixDirent dent;
    IfsMinixIo io = {io_read, NULL, sizeof disk};
    uint8_t bytes[8];
    fixture(version, name_bytes);
    if (!api || api->abi_version != IFS_MINIX_ABI_VERSION ||
        api->structure_bytes != sizeof *api ||
        api->capabilities != (IFS_MINIX_CAP_PROBE | IFS_MINIX_CAP_DIRECTORY |
                              IFS_MINIX_CAP_FILE_READ))
        return fail("ABI/capabilities mismatch");
    if (api->mount(&volume, &io) != 0 ||
        volume.version != version || volume.name_bytes != name_bytes)
        return fail("real superblock variant was rejected");
    if (api->inode(&volume, 1u, &root) != 0 ||
        api->inode(&volume, 2u, &file) != 0)
        return fail("allocated root/file inode not readable");
    if (api->entry(&volume, &root, 0u, &dent) != 0 ||
        dent.inode_number != 1u || strcmp(dent.name, ".") != 0 ||
        api->entry(&volume, &root, 2u, &dent) != 0 ||
        strcmp(dent.name, "HELLO.TXT") != 0 ||
        api->entry(&volume, &root, 3u, &dent) != 1)
        return fail("directory enumeration mismatch");
    if (api->lookup(&volume, &root, "HELLO.TXT", 9u, &dent) != 0 ||
        dent.inode_number != 2u ||
        api->lookup(&volume, &root, "hello.txt", 9u, &dent) != 1 ||
        api->lookup(&volume, &root, "wrong", 5u, &dent) != 1)
        return fail("directory lookup mismatch");
    if (api->read(&volume, &file, 7u*BS-3u, bytes, 7u) != 0 ||
        memcmp(bytes, "GGGHHHH", 7u) != 0 ||
        api->read(&volume, &file, 9u*BS+4u, bytes, 1u) != 0 ||
        bytes[0] != 'J')
        return fail("direct/single-indirect read mismatch");
    if (api->read(&volume, &file, 9u*BS+5u, bytes, 1u) == 0)
        return fail("accepted read beyond EOF");
    forged = file;
    forged.zone[7] = 65u;
    if (api->read(&volume, &forged, 7u*BS, bytes, 4u) == 0)
        return fail("accepted out-of-volume indirect zone");
    /* Holes must exist in the real inode on disk, not a forged handle. */
    if (version == 1u)
        w16(disk + 4u*BS + 32u + 14u + 7u*2u, 0u);
    else
        w32(disk + 4u*BS + 64u + 24u + 7u*4u, 0u);
    if (api->inode(&volume, 2u, &forged) != 0)
        return fail("cannot reload sparse inode");
    memset(bytes, 0xa5u, sizeof bytes);
    if (api->read(&volume, &forged, 7u*BS, bytes, 4u) != 0 ||
        bytes[0] != 0u || bytes[1] != 0u ||
        bytes[2] != 0u || bytes[3] != 0u)
        return fail("sparse hole was not zero");
    io.device_bytes = sizeof disk - BS;
    /* An image that is shorter than its on-disk declared block count fails. */
    if (api->mount(&volume, &io) == 0)
        return fail("truncated device accepted");
    io.device_bytes = sizeof disk;
    if (version == 3u) w16(disk + BS + 12u, 1u);
    else w16(disk + BS + 10u, 1u);
    if (api->mount(&volume, &io) == 0)
        return fail("unsupported zone-shift accepted");
    fixture(version, name_bytes);
    if (version == 3u) w16(disk + BS + 24u, 0u);
    else w16(disk + BS + 16u, 0u);
    if (api->mount(&volume, &io) == 0)
        return fail("unknown format magic accepted");
    fixture(version, name_bytes);
    /* Allocated flag is authoritative; an unallocated root is invalid. */
    disk[2u*BS] = 1u;
    if (api->mount(&volume, &io) == 0)
        return fail("unallocated root accepted");
    return 0;
}
int main(void)
{
    if (run_variant(1u, 14u) || run_variant(1u, 30u) ||
        run_variant(2u, 14u) || run_variant(2u, 30u) ||
        run_variant(3u, 60u)) return 1;
    printf("MINIX-V1-V2-V3-ROOT-INDIRECT-BOUNDS-PASS (%u reads)\n", reads);
    return 0;
}

/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Disposable 33-block EXT2 image. Never opens a physical disk. */
#include "../native/filesystems/ext2/module/ext2_module.h"
#include <stdio.h>
#include <string.h>

enum { BLOCK_SIZE = 1024, BLOCK_COUNT = 33 };
typedef struct {
    unsigned char bytes[BLOCK_SIZE * BLOCK_COUNT];
    unsigned writes;
    unsigned flushes;
    int fail_flush;
} MemoryDisk;

static void le16(unsigned char *p, unsigned x)
{
    p[0] = (unsigned char)x;
    p[1] = (unsigned char)(x >> 8);
}
static void le32(unsigned char *p, unsigned x)
{
    p[0] = (unsigned char)x;
    p[1] = (unsigned char)(x >> 8);
    p[2] = (unsigned char)(x >> 16);
    p[3] = (unsigned char)(x >> 24);
}
static int disk_read(void *user, uint64_t offset, void *dst, uint32_t n)
{
    MemoryDisk *d = (MemoryDisk *)user;
    if (offset > sizeof d->bytes || (uint64_t)n > sizeof d->bytes - offset)
        return -1;
    memcpy(dst, d->bytes + (size_t)offset, n);
    return 0;
}
static int disk_write(void *user, uint64_t offset, const void *src, uint32_t n)
{
    MemoryDisk *d = (MemoryDisk *)user;
    if (offset > sizeof d->bytes || (uint64_t)n > sizeof d->bytes - offset)
        return -1;
    memcpy(d->bytes + (size_t)offset, src, n);
    d->writes++;
    return 0;
}
static int disk_flush(void *user)
{
    MemoryDisk *d = (MemoryDisk *)user;
    d->flushes++;
    return d->fail_flush ? -1 : 0;
}
static int fail(const char *message)
{
    fprintf(stderr, "ext2_dropin_module_test: %s\n", message);
    return 1;
}
static void inode(unsigned char *p, unsigned mode, unsigned size, unsigned first)
{
    memset(p, 0, 128);
    le16(p, mode);
    le32(p + 0x04, size);
    le16(p + 0x1a, 1);
    le32(p + 0x28, first);
}
static void fixture(MemoryDisk *d)
{
    unsigned char *sb, *gd, *it, *dir;
    memset(d, 0, sizeof *d);
    sb = d->bytes + BLOCK_SIZE;
    gd = d->bytes + BLOCK_SIZE * 2;
    it = d->bytes + BLOCK_SIZE * 5;
    dir = d->bytes + BLOCK_SIZE * 6;

    le32(sb + 0x00, 8);
    le32(sb + 0x04, BLOCK_COUNT);
    le32(sb + 0x0c, 25);
    le32(sb + 0x10, 5);
    le32(sb + 0x14, 1);
    le32(sb + 0x18, 0);
    le32(sb + 0x1c, 0);
    le32(sb + 0x20, 32);
    le32(sb + 0x24, 32);
    le32(sb + 0x28, 8);
    le16(sb + 0x38, IFS_EXT2_SUPER_MAGIC);
    le16(sb + 0x3a, 1);
    le32(sb + 0x4c, 1);
    le32(sb + 0x54, 11);
    le16(sb + 0x58, 128);
    le32(sb + 0x60, IFS_EXT2_FEATURE_INCOMPAT_FILETYPE);
    memcpy(sb + 0x68, "0123456789abcdef", 16);
    le32(gd + 0x00, 3);
    le32(gd + 0x04, 4);
    le32(gd + 0x08, 5);
    inode(it + 128, 0x41ed, BLOCK_SIZE, 6);
    inode(it + 256, 0x81a4, 5, 7);
    le32(dir + 0, 2);
    le16(dir + 4, 12);
    dir[6] = 1;
    dir[7] = 2;
    dir[8] = '.';
    le32(dir + 12, 2);
    le16(dir + 16, 12);
    dir[18] = 2;
    dir[19] = 2;
    dir[20] = '.';
    dir[21] = '.';
    le32(dir + 24, 3);
    le16(dir + 28, 1000);
    dir[30] = 5;
    dir[31] = 1;
    memcpy(dir + 32, "hello", 5);
    memcpy(d->bytes + BLOCK_SIZE * 7, "hello", 5);
}
static int visit(void *user, uint32_t ino, IfsExt2NodeType type,
                 const char *name, uint8_t n)
{
    unsigned *seen = (unsigned *)user;
    if (ino == 3 && type == IFS_EXT2_NODE_REGULAR &&
        n == 5 && memcmp(name, "hello", 5) == 0)
        (*seen)++;
    return 0;
}

int main(void)
{
    static MemoryDisk disk;
    static IfsExt2DropInVolume volume;
    const IfsExt2DropInModule *m = infiltrator_filesystem_module_v1();
    IfsExt2DropInIo io = {disk_read, disk_write, disk_flush,
                           &disk, sizeof disk.bytes};
    unsigned found = 0;
    uint32_t ino = 0, n = 0;
    unsigned char out[8] = {0};

    if (!m || m->abi_version != 1 ||
        m->structure_bytes != sizeof *m ||
        strcmp(m->format_id, "ext2") != 0 ||
        !(m->capabilities & IFS_EXT2_CAP_WRITE_EXISTING))
        return fail("incorrect ABI or conditional-write capability");

    fixture(&disk);
    if (m->mount(&volume, &io) != IFS_EXT2_OK ||
        m->root_inode() != 2)
        return fail("failed to mount actual EXT2 image");
    if (m->lookup(&volume, 2, "hello", 5, &ino) != IFS_EXT2_OK ||
        ino != 3 ||
        m->enumerate(&volume, 2, visit, &found) != IFS_EXT2_OK ||
        found != 1)
        return fail("root enumeration/lookup incorrect");
    if (m->read(&volume, ino, 0, out, 5, &n) != IFS_EXT2_OK ||
        n != 5 || memcmp(out, "hello", 5) != 0)
        return fail("read returned incorrect contents");
    if (m->write_existing(&volume, ino, 1, "A", 1, &n) != IFS_EXT2_OK ||
        n != 1 || disk.writes != 1 || disk.flushes != 1)
        return fail("bounded in-place write did not write and flush");
    if (m->read(&volume, ino, 0, out, 5, &n) != IFS_EXT2_OK ||
        memcmp(out, "hAllo", 5) != 0)
        return fail("written bytes not readable");

    if (m->write_existing(&volume, ino, 5, "X", 1, &n) !=
        IFS_EXT2_ERROR_RANGE || disk.writes != 1)
        return fail("in-place write silently extended a file");
    if (m->write_existing(&volume, 2, 0, "X", 1, &n) !=
        IFS_EXT2_ERROR_UNSUPPORTED || disk.writes != 1)
        return fail("directory data was writable");
    if (m->read(&volume, 2, 0, out, 1, &n) !=
        IFS_EXT2_ERROR_UNSUPPORTED)
        return fail("directory exposed as a regular file");

    io.write_at = NULL;
    io.flush = NULL;
    if (m->mount(&volume, &io) != IFS_EXT2_OK ||
        m->write_existing(&volume, ino, 0, "X", 1, &n) !=
        IFS_EXT2_ERROR_UNSUPPORTED || disk.writes != 1 ||
        m->read(&volume, ino, 0, out, 5, &n) != IFS_EXT2_OK)
        return fail("read-only callback unexpectedly permitted write");
    io.write_at = disk_write;
    if (m->mount(&volume, &io) != IFS_EXT2_ERROR_ARGUMENT)
        return fail("write callback without flush was accepted");
    io.flush = disk_flush;

    io.volume_bytes = 2048;
    if (m->mount(&volume, &io) != IFS_EXT2_ERROR_RANGE)
        return fail("truncated partition bounds accepted");
    io.volume_bytes = sizeof disk.bytes;
    le16(disk.bytes + BLOCK_SIZE + 0x3a, 0);
    if (m->mount(&volume, &io) != IFS_EXT2_OK ||
        m->write_existing(&volume, ino, 0, "X", 1, &n) !=
        IFS_EXT2_ERROR_UNSUPPORTED || disk.writes != 1)
        return fail("dirty EXT2 volume permitted writes");

    fixture(&disk);
    disk.fail_flush = 1;
    if (m->mount(&volume, &io) != IFS_EXT2_OK ||
        m->write_existing(&volume, ino, 1, "B", 1, &n) !=
        IFS_EXT2_ERROR_IO || disk.flushes != 1)
        return fail("flush failure not reported");

    fixture(&disk);
    le16(disk.bytes + BLOCK_SIZE + 0x38, 0);
    if (m->mount(&volume, &io) == IFS_EXT2_OK)
        return fail("invalid EXT2 magic accepted");

    puts("EXT2-DROPIN-BOUNDED-READ-AND-EXISTING-BLOCK-WRITE-PASS");
    return 0;
}

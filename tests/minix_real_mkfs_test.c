/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Independent mkfs.minix-created volume qualification. Only image files. */
#include "minix_module.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int disk_read(void *context, uint64_t offset, void *buffer, uint32_t bytes)
{
    FILE *disk = (FILE *)context;
    if (offset > 0x7fffffffu || fseek(disk, (long)offset, SEEK_SET) != 0 ||
        fread(buffer, 1u, bytes, disk) != bytes)
        return -1;
    return 0;
}
int main(int argc, char **argv)
{
    IfsMinixIo io;
    IfsMinixVolume v;
    IfsMinixInode root;
    IfsMinixDirent dot, dotdot;
    const IfsMinixModule *api = infiltrator_filesystem_module_v1();
    FILE *disk;
    long size;
    int result = 1;
    if (argc != 4 || strlen(argv[2]) != 1u ||
        argv[2][0] < '1' || argv[2][0] > '3') return 2;
    disk = fopen(argv[1], "rb");
    if (!disk) return 1;
    if (fseek(disk, 0, SEEK_END) != 0 ||
        (size = ftell(disk)) <= 0) goto out;
    io.read_at = disk_read;
    io.user = disk;
    io.device_bytes = (uint64_t)size;
    if (!api || api->mount(&v, &io) != 0 ||
        v.version != (uint32_t)(argv[2][0] - '0') ||
        (strcmp(argv[3], "14") == 0 && v.name_bytes != 14u) ||
        (strcmp(argv[3], "30") == 0 && v.name_bytes != 30u) ||
        (strcmp(argv[3], "60") == 0 && v.name_bytes != 60u) ||
        api->inode(&v, 1u, &root) != 0 ||
        (root.mode & 0xf000u) != IFS_MINIX_MODE_DIRECTORY ||
        api->lookup(&v, &root, ".", 1u, &dot) != 0 ||
        dot.inode_number != 1u ||
        api->lookup(&v, &root, "..", 2u, &dotdot) != 0 ||
        dotdot.inode_number != 1u) {
        fprintf(stderr, "mkfs.minix V%s root test failed\n", argv[2]);
        goto out;
    }
    printf("MINIX-REAL-MKFS-V%s-NAMES%s-PASS\n", argv[2], argv[3]);
    result = 0;
out:
    fclose(disk);
    return result;
}

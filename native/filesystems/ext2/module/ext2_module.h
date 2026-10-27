/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Independent EXT2.so ABI v1. No Linux VFS, libc, UEFI or kernel dependency.
 * The caller supplies partition-relative I/O and owns the volume lifetime.
 * One operation at a time per volume: scratch space is reused.
 *
 * "WRITE_EXISTING" means in-place bytes within already allocated regular
 * file blocks; it is NOT general filesystem write/create/resize support.
 * Callers must obtain exclusive writable access and ensure recovery policy.
 */
#ifndef INFILTRATOR_EXT2_DROPIN_H
#define INFILTRATOR_EXT2_DROPIN_H

#include <stdint.h>
#include "../core/ext2_engine.h"

#define IFS_EXT2_DROPIN_ABI_VERSION 1u
#define IFS_EXT2_CAP_PROBE            UINT64_C(1)
#define IFS_EXT2_CAP_LOOKUP           UINT64_C(2)
#define IFS_EXT2_CAP_ENUMERATE        UINT64_C(4)
#define IFS_EXT2_CAP_FILE_READ        UINT64_C(8)
#define IFS_EXT2_CAP_WRITE_EXISTING   UINT64_C(16)
#define IFS_EXT2_CAP_PARTITION_IO     UINT64_C(32)

typedef int (*IfsExt2DropInReadAt)(void *, uint64_t, void *, uint32_t);
typedef int (*IfsExt2DropInWriteAt)(void *, uint64_t, const void *, uint32_t);
typedef int (*IfsExt2DropInFlush)(void *);

typedef struct IfsExt2DropInIo {
    IfsExt2DropInReadAt read_at;
    IfsExt2DropInWriteAt write_at; /* optional: null means read-only */
    IfsExt2DropInFlush flush;      /* mandatory when write_at is supplied */
    void *context;
    uint64_t volume_bytes;         /* strict partition-relative extent */
} IfsExt2DropInIo;

typedef struct IfsExt2DropInVolume {
    IfsExt2DropInIo device;
    IfsExt2Volume engine;
    uint8_t scratch[IFS_EXT2_MAX_BLOCK_SIZE];
} IfsExt2DropInVolume;

typedef struct IfsExt2DropInModule {
    uint32_t abi_version;
    uint32_t structure_bytes;
    const char *format_id;
    const char *format_name;
    uint64_t capabilities;
    IfsExt2Status (*mount)(IfsExt2DropInVolume *, const IfsExt2DropInIo *);
    uint32_t (*root_inode)(void);
    IfsExt2Status (*lookup)(IfsExt2DropInVolume *, uint32_t parent,
                             const char *, uint8_t name_length, uint32_t *found);
    IfsExt2Status (*enumerate)(IfsExt2DropInVolume *, uint32_t directory,
                                IfsExt2DirectoryCallback, void *user);
    IfsExt2Status (*read)(IfsExt2DropInVolume *, uint32_t inode,
                           uint64_t offset, void *, uint32_t requested,
                           uint32_t *bytes_read);
    IfsExt2Status (*write_existing)(IfsExt2DropInVolume *, uint32_t inode,
                                     uint64_t offset, const void *,
                                     uint32_t requested, uint32_t *bytes_written);
} IfsExt2DropInModule;

const IfsExt2DropInModule *infiltrator_filesystem_module_v1(void);
#endif

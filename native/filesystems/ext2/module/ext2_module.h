/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Standalone EXT2.so ABI. The native EXT2 engine is compiled into this
 * one module; it never depends on an installed EXT2.ko or another .so.
 * The host retains ownership of the volume and scratch memory. A mounted
 * volume's address must not change because its I/O context is embedded.
 */
#ifndef INFILTRATR_EXT2_DROPIN_MODULE_H
#define INFILTRATR_EXT2_DROPIN_MODULE_H
#include "../core/ext2_engine.h"

#define IFS_EXT2_DROPIN_ABI_VERSION 1U
#define IFS_EXT2_CAP_PROBE           UINT64_C(1)
#define IFS_EXT2_CAP_LOOKUP          UINT64_C(2)
#define IFS_EXT2_CAP_FILE_READ       UINT64_C(4)
/* Bounded overwrite only: no create, append, resize, deletion or allocation. */
#define IFS_EXT2_CAP_EXISTING_WRITE  UINT64_C(8)

typedef struct IfsExt2DropInIo {
    IfsExt2ReadAtFn read_at;
    IfsExt2WriteAtFn write_at; /* optional; must also provide flush */
    IfsExt2FlushFn flush;
    void *context;
    ifs_ext2_u64 device_bytes; /* partition length, not entire disk */
} IfsExt2DropInIo;

typedef struct IfsExt2DropInVolume {
    IfsExt2DropInIo physical;
    IfsExt2Volume engine;
} IfsExt2DropInVolume;

typedef struct IfsExt2DropInModule {
    ifs_ext2_u32 abi_version;
    ifs_ext2_u32 structure_bytes;
    const char *format_id;
    const char *format_name;
    ifs_ext2_u64 capabilities;
    IfsExt2Status (*mount)(IfsExt2DropInVolume *,
                           const IfsExt2DropInIo *);
    IfsExt2Status (*inode)(const IfsExt2DropInVolume *, ifs_ext2_u32,
                           IfsExt2Inode *, void *, ifs_ext2_u32);
    IfsExt2Status (*lookup)(const IfsExt2DropInVolume *,
                            const IfsExt2Inode *, const char *,
                            ifs_ext2_u8, IfsExt2Inode *, void *,
                            ifs_ext2_u32);
    IfsExt2Status (*read)(const IfsExt2DropInVolume *,
                          const IfsExt2Inode *, ifs_ext2_u64, void *,
                          ifs_ext2_u32, void *, ifs_ext2_u32,
                          ifs_ext2_u32 *);
    IfsExt2Status (*overwrite_existing)(const IfsExt2DropInVolume *,
                          const IfsExt2Inode *, ifs_ext2_u64, const void *,
                          ifs_ext2_u32, void *, ifs_ext2_u32,
                          ifs_ext2_u32 *);
} IfsExt2DropInModule;

const IfsExt2DropInModule *infiltrator_filesystem_module_v1(void);
#endif

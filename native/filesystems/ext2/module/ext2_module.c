/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Self-contained EXT2.so backed by the existing canonical EXT2 engine.
 * Read/write operations are partition-bounded and keep format checks inside
 * the engine. In-place overwrites require a clean EXT2 volume, a writable
 * device and flush support. No journal replay, allocation or defrag.
 */
#include "ext2_module.h"

static int bounded_read(void *user, ifs_ext2_u64 offset,
                        void *data, ifs_ext2_u32 count)
{
    const IfsExt2DropInIo *io = (const IfsExt2DropInIo *)user;
    if (!io || !io->read_at || !data ||
        offset > io->device_bytes ||
        (ifs_ext2_u64)count > io->device_bytes - offset)
        return -1;
    return io->read_at(io->context, offset, data, count);
}

static int bounded_write(void *user, ifs_ext2_u64 offset,
                         const void *data, ifs_ext2_u32 count)
{
    const IfsExt2DropInIo *io = (const IfsExt2DropInIo *)user;
    if (!io || !io->write_at || !io->flush || !data ||
        offset > io->device_bytes ||
        (ifs_ext2_u64)count > io->device_bytes - offset)
        return -1;
    return io->write_at(io->context, offset, data, count);
}

static IfsExt2Status mount_volume(IfsExt2DropInVolume *v,
                                  const IfsExt2DropInIo *io)
{
    IfsExt2Io adapter;
    IfsExt2Status status;
    ifs_ext2_u32 risks = 0;
    if (!v || !io || !io->read_at || io->device_bytes < 2048U)
        return IFS_EXT2_ERROR_ARGUMENT;

    v->physical = *io;
    adapter.read_at = bounded_read;
    adapter.write_at = (io->write_at && io->flush) ? bounded_write : IFS_EXT2_NULL;
    adapter.flush = IFS_EXT2_NULL; /* per-operation flush uses host callback */
    adapter.user = &v->physical;
    status = ifs_ext2_open(&v->engine, &adapter);
    if (status != IFS_EXT2_OK)
        return status;
    if (v->engine.byte_size > io->device_bytes)
        return IFS_EXT2_ERROR_RANGE;
    /* Never expose a dirty/error-state volume as safely mountable. */
    return ifs_ext2_readonly_assess(&v->engine, &risks);
}

static IfsExt2Status read_inode(const IfsExt2DropInVolume *v,
                                ifs_ext2_u32 inode, IfsExt2Inode *out,
                                void *scratch, ifs_ext2_u32 scratch_bytes)
{
    if (!v)
        return IFS_EXT2_ERROR_ARGUMENT;
    return ifs_ext2_read_inode(&v->engine, inode, out, scratch, scratch_bytes);
}

static IfsExt2Status lookup_inode(const IfsExt2DropInVolume *v,
                                  const IfsExt2Inode *parent,
                                  const char *name, ifs_ext2_u8 length,
                                  IfsExt2Inode *out, void *scratch,
                                  ifs_ext2_u32 scratch_bytes)
{
    ifs_ext2_u32 inode_number = 0;
    IfsExt2Status status;
    if (!v || !parent || !name || !out || !length)
        return IFS_EXT2_ERROR_ARGUMENT;
    status = ifs_ext2_lookup(&v->engine, parent, name, length,
                            &inode_number, scratch, scratch_bytes);
    if (status != IFS_EXT2_OK)
        return status;
    return ifs_ext2_read_inode(&v->engine, inode_number, out,
                               scratch, scratch_bytes);
}

static IfsExt2Status read_file(const IfsExt2DropInVolume *v,
                               const IfsExt2Inode *inode,
                               ifs_ext2_u64 offset, void *buffer,
                               ifs_ext2_u32 bytes, void *scratch,
                               ifs_ext2_u32 scratch_bytes,
                               ifs_ext2_u32 *read_bytes)
{
    if (!v || ifs_ext2_inode_type(inode) != IFS_EXT2_NODE_REGULAR)
        return IFS_EXT2_ERROR_ARGUMENT;
    return ifs_ext2_read_file(&v->engine, inode, offset, buffer, bytes,
                              scratch, scratch_bytes, read_bytes);
}

static IfsExt2Status overwrite_existing(const IfsExt2DropInVolume *v,
                               const IfsExt2Inode *inode,
                               ifs_ext2_u64 offset, const void *buffer,
                               ifs_ext2_u32 bytes, void *scratch,
                               ifs_ext2_u32 scratch_bytes,
                               ifs_ext2_u32 *written)
{
    IfsExt2Status status;
    if (written)
        *written = 0U;
    if (!v || !inode || !written || !v->physical.write_at ||
        !v->physical.flush)
        return IFS_EXT2_ERROR_UNSUPPORTED;
    status = ifs_ext2_write_file_existing(&v->engine, inode, offset,
               buffer, bytes, scratch, scratch_bytes, written);
    if (status != IFS_EXT2_OK)
        return status;
    /* A failed flush is a failed operation even if some sectors changed.
     * Callers must treat I/O errors as possibly partially committed writes.
     */
    return v->physical.flush(v->physical.context) == 0
        ? IFS_EXT2_OK : IFS_EXT2_ERROR_IO;
}

__attribute__((used, section(".ifsmeta")))
static const char format_identity[] = "IFS-MODULE:ext2:1";

static const IfsExt2DropInModule module = {
    IFS_EXT2_DROPIN_ABI_VERSION, sizeof(IfsExt2DropInModule),
    "ext2", "Second Extended File System",
    IFS_EXT2_CAP_PROBE | IFS_EXT2_CAP_LOOKUP |
    IFS_EXT2_CAP_FILE_READ | IFS_EXT2_CAP_EXISTING_WRITE,
    mount_volume, read_inode, lookup_inode, read_file, overwrite_existing
};

__attribute__((visibility("default")))
const IfsExt2DropInModule *infiltrator_filesystem_module_v1(void)
{
    return &module;
}

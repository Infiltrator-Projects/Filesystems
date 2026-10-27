/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * EXT2.so is a thin freestanding wrapper around the canonical EXT2 engine.
 * No second EXT2 parser or hidden Linux package/kernel/EFI integration.
 */
#include "ext2_module.h"

static int in_bounds(const IfsExt2DropInVolume *v,
                     uint64_t offset, uint32_t count)
{
    return v && offset <= v->device.volume_bytes &&
           (uint64_t)count <= v->device.volume_bytes - offset;
}

static int bounded_read(void *user, uint64_t offset,
                        void *target, uint32_t count)
{
    IfsExt2DropInVolume *v = (IfsExt2DropInVolume *)user;
    if (!in_bounds(v, offset, count) || !v->device.read_at ||
        (count && !target))
        return -1;
    return v->device.read_at(v->device.context, offset, target, count);
}

static int bounded_write(void *user, uint64_t offset,
                         const void *source, uint32_t count)
{
    IfsExt2DropInVolume *v = (IfsExt2DropInVolume *)user;
    if (!in_bounds(v, offset, count) || !v->device.write_at ||
        (count && !source))
        return -1;
    return v->device.write_at(v->device.context, offset, source, count);
}

static int bounded_flush(void *user)
{
    IfsExt2DropInVolume *v = (IfsExt2DropInVolume *)user;
    return v && v->device.flush ?
           v->device.flush(v->device.context) : -1;
}

static IfsExt2Status module_mount(IfsExt2DropInVolume *v,
                                   const IfsExt2DropInIo *device)
{
    IfsExt2Io engine_io;
    IfsExt2Status status;
    if (!v || !device || !device->read_at ||
        device->volume_bytes < 2048u ||
        (device->write_at && !device->flush))
        return IFS_EXT2_ERROR_ARGUMENT;
    v->device = *device;
    engine_io.read_at = bounded_read;
    engine_io.write_at = device->write_at ? bounded_write : 0;
    engine_io.flush = device->write_at ? bounded_flush : 0;
    engine_io.user = v;
    status = ifs_ext2_open(&v->engine, &engine_io);
    if (status != IFS_EXT2_OK)
        return status;
    if (v->engine.byte_size > device->volume_bytes)
        return IFS_EXT2_ERROR_RANGE;
    return IFS_EXT2_OK;
}

static uint32_t module_root_inode(void)
{
    return IFS_EXT2_ROOT_INODE;
}

static IfsExt2Status module_lookup(IfsExt2DropInVolume *v,
                                    uint32_t parent, const char *name,
                                    uint8_t name_length, uint32_t *found)
{
    IfsExt2Inode dir;
    IfsExt2Status status;
    if (!v || !name || name_length == 0u || !found)
        return IFS_EXT2_ERROR_ARGUMENT;
    status = ifs_ext2_read_inode(&v->engine, parent, &dir,
                                 v->scratch, sizeof v->scratch);
    if (status != IFS_EXT2_OK)
        return status;
    return ifs_ext2_lookup(&v->engine, &dir, name, name_length,
                            found, v->scratch, sizeof v->scratch);
}

static IfsExt2Status module_enumerate(IfsExt2DropInVolume *v,
                                       uint32_t directory,
                                       IfsExt2DirectoryCallback callback,
                                       void *user)
{
    IfsExt2Inode dir;
    IfsExt2Status status;
    if (!v || !callback)
        return IFS_EXT2_ERROR_ARGUMENT;
    status = ifs_ext2_read_inode(&v->engine, directory, &dir,
                                 v->scratch, sizeof v->scratch);
    if (status != IFS_EXT2_OK)
        return status;
    return ifs_ext2_iterate_directory(&v->engine, &dir, callback,
                                      user, v->scratch, sizeof v->scratch);
}

static IfsExt2Status module_read(IfsExt2DropInVolume *v,
                                  uint32_t inode_number, uint64_t offset,
                                  void *destination, uint32_t requested,
                                  uint32_t *read_count)
{
    IfsExt2Inode inode;
    IfsExt2Status status;
    if (!v || !destination || !read_count)
        return IFS_EXT2_ERROR_ARGUMENT;
    *read_count = 0u;
    status = ifs_ext2_read_inode(&v->engine, inode_number, &inode,
                                 v->scratch, sizeof v->scratch);
    if (status != IFS_EXT2_OK)
        return status;
    if (ifs_ext2_inode_type(&inode) != IFS_EXT2_NODE_REGULAR)
        return IFS_EXT2_ERROR_UNSUPPORTED;
    return ifs_ext2_read_file(&v->engine, &inode, offset,
                              destination, requested, v->scratch,
                              sizeof v->scratch, read_count);
}

static IfsExt2Status module_write_existing(IfsExt2DropInVolume *v,
                                            uint32_t inode_number,
                                            uint64_t offset, const void *source,
                                            uint32_t requested,
                                            uint32_t *written)
{
    IfsExt2Inode inode;
    IfsExt2Status status;
    if (!v || !source || !written)
        return IFS_EXT2_ERROR_ARGUMENT;
    *written = 0u;
    if (!v->device.write_at || !v->device.flush)
        return IFS_EXT2_ERROR_UNSUPPORTED;
    status = ifs_ext2_read_inode(&v->engine, inode_number, &inode,
                                 v->scratch, sizeof v->scratch);
    if (status != IFS_EXT2_OK)
        return status;
    status = ifs_ext2_write_file_existing(&v->engine, &inode, offset,
                                          source, requested, v->scratch,
                                          sizeof v->scratch, written);
    if (status != IFS_EXT2_OK)
        return status;
    if (*written && bounded_flush(v) != 0)
        return IFS_EXT2_ERROR_IO; /* bytes may already have been written */
    return IFS_EXT2_OK;
}

/* Non-executed format metadata allows discovery without dlopen. */
__attribute__((used,section(".ifsmeta")))
static const char module_format_identity[] = "IFS-MODULE:ext2:1";

static const IfsExt2DropInModule module = {
    IFS_EXT2_DROPIN_ABI_VERSION,
    sizeof(IfsExt2DropInModule),
    "ext2",
    "Second Extended File System",
    IFS_EXT2_CAP_PROBE | IFS_EXT2_CAP_LOOKUP |
    IFS_EXT2_CAP_ENUMERATE | IFS_EXT2_CAP_FILE_READ |
    IFS_EXT2_CAP_WRITE_EXISTING | IFS_EXT2_CAP_PARTITION_IO,
    module_mount, module_root_inode, module_lookup, module_enumerate,
    module_read, module_write_existing
};

__attribute__((visibility("default")))
const IfsExt2DropInModule *infiltrator_filesystem_module_v1(void)
{
    return &module;
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include "infiltratr/fs/byte_reader.h"
#include <infiltratr/filesystem_module_v2.h>
#include "infiltratr/fs/io.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    const unsigned char *bytes;
    size_t length;
} MemoryIo;

static IfsStatus memory_read(void *context,
                             const ifs_u64 offset,
                             void *buffer,
                             const size_t length)
{
    const MemoryIo *memory = context;

    if (memory == NULL || buffer == NULL ||
        offset > memory->length ||
        (ifs_u64)length > (ifs_u64)memory->length - offset)
        return IFS_ERROR_RANGE;

    memcpy(buffer, memory->bytes + (size_t)offset, length);
    return IFS_OK;
}

static int fail(const char *message)
{
    fprintf(stderr, "native_core_test: %s\n", message);
    return 1;
}

/* Filesystems does not define its own filesystem ABI. Common V2 is the
 * single provider descriptor and capability validator for all consumers. */
static int v2_probe(const IfsFsHostRuntimeV2 *host, uint32_t *variant)
{
    if (!host || !variant || !ifs_fs_host_v2_valid(host))
        return IFS_FS_INVALID;
    *variant=0u;
    return IFS_FS_NOT_FOUND;
}
static IfsFsCoreOpsV2 v2_core={
    .header={IFS_FS_GROUP_ABI_V2,sizeof(IfsFsCoreOpsV2),
             IFS_FS_GROUP_CORE,0u,IFS_FS_CORE_PROBE},
    .probe=v2_probe
};
static const IfsFsGroupHeaderV2 *v2_query(uint32_t id,
                                          uint32_t minimum,uint32_t maximum)
{
    return id==IFS_FS_GROUP_CORE &&
           minimum<=IFS_FS_GROUP_ABI_V2 &&
           maximum>=IFS_FS_GROUP_ABI_V2 ? &v2_core.header : NULL;
}

int main(void)
{
    static const unsigned char bytes[] = {
        0x11U, 0x22U, 0x33U, 0x44U,
        0x55U, 0x66U, 0x77U, 0x88U
    };
    MemoryIo memory = {bytes, sizeof(bytes)};
    IfsIo io = {
        .context = &memory,
        .size_bytes = sizeof(bytes),
        .flags = IFS_IO_READABLE,
        .read_at = memory_read,
        .write_at = NULL
    };
    unsigned char copied[4] = {0};
    IfsByteReader reader;
    const ifs_u8 *taken = NULL;
    ifs_u16 be16 = 0U;
    ifs_u32 le32 = 0U;
    IfsFsModuleApiV2 descriptor={
        .abi_version=IFS_FS_COMMON_ABI_V2,
        .structure_bytes=sizeof(IfsFsModuleApiV2),
        .format_id="test",
        .runtime_profiles=IFS_FS_PROFILE_HOSTED,
        .group_mask=IFS_FS_GROUP_BIT(IFS_FS_GROUP_CORE),
        .query_group=v2_query
    };

    if (!ifs_io_is_valid(&io))
        return fail("valid memory I/O rejected");
    if (ifs_io_read_exact(&io, 2U, copied, sizeof(copied)) != IFS_OK ||
        memcmp(copied, bytes + 2U, sizeof(copied)) != 0)
        return fail("exact bounded read failed");
    if (ifs_io_read_exact(&io, 7U, copied, 2U) != IFS_ERROR_RANGE)
        return fail("out-of-range read was not rejected");
    if (ifs_io_write_exact(&io, 0U, copied, 1U) != IFS_ERROR_READ_ONLY)
        return fail("read-only I/O accepted a write");

    ifs_byte_reader_init(&reader, bytes, sizeof(bytes));
    if (ifs_byte_reader_be16(&reader, &be16) != IFS_OK ||
        be16 != 0x1122U)
        return fail("big-endian decode failed");
    if (ifs_byte_reader_le32(&reader, &le32) != IFS_OK ||
        le32 != 0x66554433U)
        return fail("little-endian decode failed");
    if (ifs_byte_reader_skip(&reader, 3U) != IFS_ERROR_RANGE)
        return fail("reader overrun was not rejected");

    /* A zero-length read over an empty reader must not perform pointer
     * arithmetic on a null storage base. The result is an empty span. */
    ifs_byte_reader_init(&reader, NULL, sizeof(bytes));
    taken = (const ifs_u8 *)bytes;
    if (ifs_byte_reader_take(&reader, 0U, &taken) != IFS_OK || taken != NULL)
        return fail("empty reader zero-length take was not deterministic");

    /* The reader is a public value type. Reject externally corrupted cursor
     * state even when the requested operation is zero bytes. */
    reader.cursor = 1U;
    if (ifs_byte_reader_skip(&reader, 0U) != IFS_ERROR_RANGE)
        return fail("invalid reader cursor accepted zero-length skip");
    if (ifs_byte_reader_take(&reader, 0U, &taken) != IFS_ERROR_RANGE)
        return fail("invalid reader cursor accepted zero-length take");

    if (!ifs_fs_api_v2_valid(&descriptor, "test"))
        return fail("valid Common V2 descriptor rejected");
    v2_core.probe=NULL;
    if (ifs_fs_api_v2_valid(&descriptor,"test"))
        return fail("V2 probe capability accepted without callback");
    v2_core.probe=v2_probe;
    v2_core.header.abi_version=1u;
    if (ifs_fs_api_v2_valid(&descriptor,"test"))
        return fail("retired group revision accepted");
    v2_core.header.abi_version=IFS_FS_GROUP_ABI_V2;
    if (!ifs_fs_api_v2_valid(&descriptor,"test"))
        return fail("valid V2 descriptor was not restored");

    return 0;
}

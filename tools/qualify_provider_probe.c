/* SPDX-License-Identifier: GPL-3.0-or-later
 * Exercise the published or source-built provider through the canonical ABI.
 * The negative fixture is a private sparse regular file. A positive fixture
 * must be supplied explicitly and is opened read-only. A strict host records
 * alignment, bounds and allocator balance independently of the provider.
 */
#define _POSIX_C_SOURCE 200809L
#define _FILE_OFFSET_BITS 64
#include <infiltratr/filesystem_module_v2.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    int fd;
    uint64_t bytes, reads;
    size_t allocations;
    unsigned sector, invalid;
} ProbeHost;

static int read_sectors(void *context, uint64_t offset, void *buffer, uint32_t size)
{
    ProbeHost *host = context;
    if (!size || size > 65536u || offset % host->sector || size % host->sector ||
        offset > host->bytes || size > host->bytes - offset) {
        ++host->invalid;
        return -1;
    }
    ++host->reads;
    ssize_t got;
    do {
        got = pread(host->fd, buffer, size, (off_t)offset);
    } while (got < 0 && errno == EINTR);
    return got == size ? 0 : -1;
}

static void *allocate(void *context, size_t size)
{
    ProbeHost *host = context;
    void *p = malloc(size);
    if (p) ++host->allocations;
    return p;
}

static void release(void *context, void *p)
{
    ProbeHost *host = context;
    if (!p) return;
    if (host->allocations) --host->allocations;
    else ++host->invalid;
    free(p);
}

static int exercise(const IfsFsModuleApiV2 *api, int fd, uint64_t bytes,
                    uint32_t sector, int expect_match)
{
    ProbeHost fixture = {.fd = fd, .bytes = bytes, .sector = sector};
    IfsFsHostRuntimeV2 host = {
        .abi_version = IFS_FS_HOST_ABI_V2, .structure_bytes = sizeof(host),
        .capabilities = IFS_FS_HOST_READ_ONLY, .volume_bytes = bytes,
        .logical_sector_bytes = sector, .physical_sector_bytes = 4096u,
        .maximum_transfer_bytes = 65536u, .context = &fixture,
        .read_at = read_sectors, .allocate = allocate, .release = release
    };
    const IfsFsCoreOpsV2 *core = (const IfsFsCoreOpsV2 *)
        ifs_fs_group_v2(api, IFS_FS_GROUP_CORE);
    if (!core || !(core->header.capabilities & IFS_FS_CORE_PROBE) || !core->probe)
        return 1;
    uint32_t variant = 0;
    int result = core->probe(&host, &variant);
    printf("%s sector=%" PRIu32 " result=%d variant=%" PRIu32
           " reads=%" PRIu64 " violations=%u outstanding=%zu\n",
           api->format_id, sector, result, variant, fixture.reads,
           fixture.invalid, fixture.allocations);
    int expected = expect_match ? result == IFS_FS_OK :
        (result == IFS_FS_NOT_FOUND || result == IFS_FS_CORRUPT ||
         result == IFS_FS_UNSUPPORTED);
    return !expected || !fixture.reads || fixture.invalid || fixture.allocations;
}

int main(int argc, char **argv)
{
    int custom_blank = argc == 5 && strcmp(argv[3], "--blank-bytes") == 0;
    if (argc != 3 && argc != 4 && !custom_blank) {
        fputs("usage: qualify_provider_probe MODULE.so FORMAT_ID [KNOWN_IMAGE | --blank-bytes SIZE]\n", stderr);
        return 2;
    }
    void *lib = dlopen(argv[1], RTLD_LOCAL | RTLD_NOW);
    if (!lib) { fprintf(stderr, "%s\n", dlerror()); return 2; }
    void *sym = dlsym(lib, IFS_FS_COMMON_EXPORT_V2);
    IfsFsModuleApiEntryV2 entry;
    _Static_assert(sizeof(entry) == sizeof(sym), "incompatible dynamic symbol representation");
    memcpy(&entry, &sym, sizeof(entry));
    const IfsFsModuleApiV2 *api = entry ? entry() : NULL;
    if (!api || !ifs_fs_api_v2_valid(api, argv[2])) { dlclose(lib); return 1; }
    printf("groups");
    for (unsigned i = 1; i <= IFS_FS_GROUP_MAX; ++i) {
        const IfsFsGroupHeaderV2 *g = ifs_fs_group_v2(api, i);
        printf(" %" PRIu64, g ? g->capabilities : UINT64_C(0));
    }
    putchar('\n');
    FILE *blank = NULL;
    int fd = -1;
    if (argc == 4) fd = open(argv[3], O_RDONLY);
    else {
        off_t size = INT64_C(1073741824);
        if (custom_blank) {
            char *end;
            errno = 0;
            intmax_t value = strtoimax(argv[4], &end, 10);
            if (errno || *end || value < 4096 || value % 4096 ||
                (intmax_t)(off_t)value != value) { dlclose(lib); return 2; }
            size = (off_t)value;
        }
        blank = tmpfile();
        if (blank && ftruncate(fileno(blank), size) == 0)
            fd = fileno(blank);
    }
    struct stat info;
    int failed = 1;
    if (fd >= 0 && fstat(fd, &info) == 0 && S_ISREG(info.st_mode) && info.st_size > 0) {
        int result_512 = exercise(api, fd, (uint64_t)info.st_size, 512u, argc == 4);
        int result_4096 = exercise(api, fd, (uint64_t)info.st_size, 4096u, argc == 4);
        failed = result_512 || result_4096;
    }
    if (blank) fclose(blank); else if (fd >= 0) close(fd);
    dlclose(lib);
    return failed;
}

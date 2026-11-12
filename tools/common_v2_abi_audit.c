/* SPDX-License-Identifier: GPL-3.0-or-later
 * Audit a *built* independent filesystem provider, not its claimed source ABI.
 * This read-only inspector dlopens one SO in an isolated CI process, checks
 * native Common V2 structural invariants, and prints its truthful abilities.
 * It never calls probe, mount, format, plan, execute, verify or recover.
 */
#define _GNU_SOURCE
#include <infiltratr/filesystem_module_v2.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    void *library, *symbol;
    IfsFsModuleApiEntryV2 entry = NULL;
    const IfsFsModuleApiV2 *api;
    const IfsFsGroupHeaderV2 *core, *volume, *defrag;
    if (argc != 2 || !argv[1][0]) {
        fputs("usage: common_v2_abi_audit PROVIDER.so\n", stderr);
        return 2;
    }
    library = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL);
    if (!library) {
        fprintf(stderr, "SO_LOAD_FAILED: %s\n", dlerror());
        return 3;
    }
    symbol = dlsym(library, IFS_FS_COMMON_EXPORT_V2);
    if (!symbol) {
        fputs("NO_COMMON_V2_ENTRY\n", stderr);
        dlclose(library);
        return 4;
    }
    _Static_assert(sizeof(entry) == sizeof(symbol),
                   "Common V2 function-pointer ABI size mismatch");
    memcpy(&entry, &symbol, sizeof(entry));
    api = entry ? entry() : NULL;
    if (!api || !ifs_fs_api_v2_valid(api, api->format_id)) {
        fputs("INVALID_COMMON_V2_ABI\n", stderr);
        dlclose(library);
        return 5;
    }
    core = ifs_fs_group_v2(api, IFS_FS_GROUP_CORE);
    volume = ifs_fs_group_v2(api, IFS_FS_GROUP_VOLUME);
    defrag = ifs_fs_group_v2(api, IFS_FS_GROUP_DEFRAG);
    printf("%s\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\t%" PRIu64 "\n",
           api->format_id, api->runtime_profiles,
           core ? core->capabilities : UINT64_C(0),
           volume ? volume->capabilities : UINT64_C(0),
           defrag ? defrag->capabilities : UINT64_C(0));
    dlclose(library);
    return 0;
}

# Filesystem .so ABI audit and first unification pass — 30 October 2026

## Scope and finding

Inspected all six independent `Filesystem-*` GitHub repositories, the Filesystems CMake standalone module targets, their public C module headers/entry points, the Filesystems 41-format catalogue and the project-owned InfiltratorFS tree.

**Nine distinct implemented standalone .so formats have been identified** in these sources: FAT12, FAT16, FAT32, OFS, FFS, exFAT, EXT2, SCO BFS and Minix. The remaining Filesystems catalogue formats **must not be described as having a unified standalone .so** merely because a Linux kernel driver, utility or candidate format is available. InfiltratorFS currently supplies its own filesystem core and platform integrations, not a qualified instance of the new common .so entry point.

## What is already common

Across the nine existing standalone modules, exported `infiltrator_filesystem_module_vN()` tables begin with:
```c
uint32_t abi_version;
uint32_t structure_bytes;
const char *format_id;
```
The exported symbol names, the volume-relative bounded read callback pattern, and zero/nonzero error conventions are similar. Existing tables **do not** share a safe common capability mask, operation signatures, volume object layout, ownership model, formatter contract, directory enumeration API or write authority. In particular, `IFS_*_CAP_*` masks are local to their native ABI even when their integer values coincide.

| Existing .so identity | Source / latest observed entry point | Shared operations seen in its native ABI | Important non-uniformity |
|---|---|---|---|
| FAT12 | Filesystem-FAT12, V1/V3/V4 | Probe, root listing/read; newer path, write and format APIs | FAT12-only geometry, vintage V1 root/8.3 API, later FatFs calls |
| FAT16 | Filesystem-FAT16, V1/V3/V4/V5 | Probe, root listing/read; newer path, write/format/metadata APIs | Same classic V1 shape as FAT12; V5 metadata extension |
| FAT32 | Filesystem-FAT32, V1/V3/V4 | Probe, root listing/read; newer path, write and format APIs | V1 root/8.3 API; FAT32 allocation rules |
| OFS | Filesystem-OFS, V1/V2/V3 | Mount/lookup/list/read; V2 bounded overwrite; V3 extended write/format/check/recover | Per-disk DOS0/2/4/6 variants and name limits; V3 uses a different I/O and volume lifecycle |
| FFS | Filesystem-FFS, V1/V2 | Mount/lookup/list/read, V2 allocated-data overwrite | FFS data block semantics; narrower V2 write surface |
| exFAT | Filesystem-exFAT, V3/V4/V5 | Mount/list/read/write/create/rename/format; V5 64-bit file operations | Per-module FatFs state, V5 delegates to V4; no generic host volume handle |
| EXT2 | Filesystems, V1 | Mount/lookup/list/read, bounded existing-file overwrite | Requires caller scratch buffers/inode objects |
| SCO BFS | Filesystems, V1 | Mount/root listing/lookup/read, bounded overwrite | SCO BFS, **not** BeOS BeFS |
| Minix | Filesystems, V1/V2 | Mount/list/lookup/read, bounded overwrite | Minix V1/V2/V3 on-disk variants; V2 has additional write I/O |

This is an **inspection of source interfaces**, not a claim that every advertised operation passes an independent real-media test.

## What changed in Common / Libraries

- The new format-neutral `include/infiltratr/filesystem_module.h` defines identical new host I/O callbacks, status values, feature bits, per-mounted-volume variant information, opaque volume ownership and optional operation signatures for any conforming format.
- The interface explicitly distinguishes `format_id` from a disk's detected `variant`. An OFS DOS0 volume therefore cannot inherit DOS6 filename rules merely because one library supports both.
- The new `include/infiltratr/filesystem_module_loader.h` and `src/filesystem_module_loader.c` provide a format-neutral **actual loaded-module inspection** function. It accepts only an already opened/trust-verified module handle, checks for `infiltrator_filesystem_api_v1`, or falls back to the highest compatible existing `infiltrator_filesystem_module_vN` table (V1 through V8). It validates ABI version, table size and requested format identity. For legacy modules it returns an opaque native table; it never invents a common capability mask or calls incompatible native methods.
- `tests/filesystem_module_smoke.c` and `tests/filesystem_module_loader_smoke.c`, with both common/legacy compiled fixtures, are wired into Libraries CMake tests. Invalid identities, mismatched versions, missing capability callbacks and undeclared defrag operations fail closed.

## Qualification boundary

**The common discovery/validation layer is now implemented at the owner of shared contracts. Universal read/write/format calls are defined but have not been adopted by all nine producers or consumers.** A consumer cannot yet treat a legacy OFS native table as a FAT32 operation table. The new common API must be implemented by per-format adapters calling the existing canonical engines, then tested against media and existing callers. Legacy exports must remain available until parity is proved.

The new contract introduces no new per-format dependency or duplicate filesystem engine, and does not install or execute arbitrary dropped-in modules during metadata scan. `.ko`, `.efi` and BOOT/kernel module execution remain separately qualified runtime integration problems.

Cross-references:
- [Authoritative architecture](UNIFIED_FILESYSTEM_MODULE_ARCHITECTURE.md)
- [Common's ABI owner](https://github.com/Infiltrator-Projects/Libraries/blob/main/include/infiltratr/filesystem_module.h)
- [Common's already-loaded compatibility inspector](https://github.com/Infiltrator-Projects/Libraries/blob/main/src/filesystem_module_loader.c)

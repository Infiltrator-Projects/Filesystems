# Filesystem extraction inventory — 30 October 2026

This document records the initial, non-destructive extraction of Defragmenter/Filesystems source code into the 16 newly created `Filesystem-*` repositories. Existing application and kernel paths have NOT been changed and are still authoritative at runtime.

**Sources pinned for the initial imports:**
- Filesystems: `ca2fa1210c3c1f2b6f52cd15b3da6dfa83d453ad`
- Defragmenter: `30af892d8897d3aeaeefeac1bfcdb9620a0a5c72`

| New repository | Source imported | Independent .so build | Existing regression in new repo |
|---|---|---|---|
| [Filesystem-NTFS](https://github.com/Infiltrator-Projects/Filesystem-NTFS) | Defragmenter NTFS native engine; Filesystems design | No | No |
| [Filesystem-EXT2](https://github.com/Infiltrator-Projects/Filesystem-EXT2) | Filesystems canonical core, V1 module + synthetic-volume test | Yes — EXT2.so | GitHub CI passed |
| [Filesystem-EXT3](https://github.com/Infiltrator-Projects/Filesystem-EXT3) | Filesystems EXT3 format core | No | No |
| [Filesystem-EXT4](https://github.com/Infiltrator-Projects/Filesystem-EXT4) | Filesystems EXT4 format core; shared EXT2/3/4 Defragmenter relocation snapshot | No | No |
| [Filesystem-XFS](https://github.com/Infiltrator-Projects/Filesystem-XFS) | Defragmenter XFS native engine; Filesystems design | No | No |
| [Filesystem-Btrfs](https://github.com/Infiltrator-Projects/Filesystem-Btrfs) | Defragmenter Btrfs native engine | No | No |
| [Filesystem-APFS](https://github.com/Infiltrator-Projects/Filesystem-APFS) | Defragmenter APFS native engine | No | No |
| [Filesystem-HFS](https://github.com/Infiltrator-Projects/Filesystem-HFS) | Defragmenter classic HFS analysis and writer | No | No |
| [Filesystem-HFSPlus](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus) | Defragmenter HFS+/HFSX native engine | No | No |
| [Filesystem-SFS](https://github.com/Infiltrator-Projects/Filesystem-SFS) | Filesystems SFS0 core; combined SFS0/SFS2 Defragmenter relocation reference | No | No |
| [Filesystem-SFS2](https://github.com/Infiltrator-Projects/Filesystem-SFS2) | Filesystems distinct SFS2 core/design | No | No |
| [Filesystem-PFS3](https://github.com/Infiltrator-Projects/Filesystem-PFS3) | Filesystems PFS3 core; Defragmenter PFS3 native engine | No | No |
| [Filesystem-Minix](https://github.com/Infiltrator-Projects/Filesystem-Minix) | Filesystems V1/V2 standalone module + test; Defragmenter Minix engine reference | Yes — MINIX.so | GitHub CI passed |
| [Filesystem-UFS](https://github.com/Infiltrator-Projects/Filesystem-UFS) | Defragmenter UFS1/UFS2 native engine | No | No |
| [Filesystem-ZFS](https://github.com/Infiltrator-Projects/Filesystem-ZFS) | Defragmenter bounded ZFS member analysis | No | No |
| [Filesystem-Swap](https://github.com/Infiltrator-Projects/Filesystem-Swap) | Defragmenter swap-space analysis | No | No |

The six independently developed repositories already present were FAT12, FAT16, FAT32, OFS, FFS and exFAT; they were not recreated in this extraction.

**Meaning of 'No'**: imported source exists, but standalone linking/qualification and cross-environment loader integration are not complete. In particular, a source copy is not an independently working `.so`.

## Rules for subsequent migration

1. Independent format repositories own the authoritative on-disk parser, lifecycle, format, recovery, and defrag implementations. Common / Libraries owns cross-format ABI definitions and host-neutral contracts, never a second format engine.
2. Do not move or delete the old Filesystems/Defragmenter runtime implementation until an independent build, media tests, error-case checks, write-safety checks, and measured dispatch parity pass in the new repository and consuming application.
3. Maintain format identity: EXT2/3/4 and SFS0/SFS2 are distinct; OFS variants stay inside OFS.so with disk-version-specific rules. Preserve separate FAT12/16/32.so products.
4. Defragmenter worker logic in `migration/defragmenter/` is temporary source provenance. Refactor filesystem logic into the canonical engine and keep host orchestration in Defragmenter. Never ship two format engines as competing implementations.
5. Swap has no regular user files. ZFS currently supports bounded analysis only. Neither gains defragmentation or arbitrary write capability by acquiring its own repository.
6. Preserve license/provenance and review original-source permissions before promoting a combined executable. Validate ABI/public metadata using Common, but never assume a conventional Linux .so can directly execute as a .ko or EFI image.

## Readiness

EXT2 and Minix passed a GitHub Actions `make test` on the newly extracted source. This is stronger than a mere source copy, but does not yet demonstrate Common ABI adoption, Defragmenter dispatch through the new module, BOOT/kernel execution, or full read/write/format/defragmenter lifecycle.

As of this extraction no changes were made to the original Defragmenter runtime dispatch or Filesystems builds.

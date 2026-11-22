# Current Common V2 filesystem work — 22 November 2026

## Qualification boundaries

The canonical interface is [Libraries Common V2](https://github.com/Infiltrator-Projects/Libraries/blob/main/include/infiltratr/filesystem_module_v2.h). Every source-tree provider currently exposes `infiltrator_filesystem_api_v2`. This document records **current source capabilities**, actual latest **GitHub release versions**, and observable tests. It does not equate compilation with a successful boot.

**BOOT is being migrated separately and was not modified.** Its final authenticated Common V2 loader and OVMF kernel/initramfs handoff have not been exercised against these released modules, so no boot-capable provider is yet marked end-to-end accepted. Firmware “namespace” means the source advertises and tests the genuine V2 PROBE/MOUNT/STAT/LIST/READ callbacks on supported on-disk features; `SWAP` must not implement a fake namespace. Historical machine-specific validator observations are intentionally excluded.

## Module-by-module progress

| Repository | ABI on main | Hosted Common V2 | Freestanding Common V2 | Published tag | Actual tests | Remaining implementation / acceptance gap |
|---|---|---|---|---|---|---|---|
| [Filesystem-APFS](https://github.com/Infiltrator-Projects/Filesystem-APFS) | V2 | Namespace, flat-tree subset | Namespace, flat-tree subset | v1.0.13 | Package + embedded reader tests green | Multilevel catalogue trees, unsupported APFS features; future V2 BOOT |
| [Filesystem-Btrfs](https://github.com/Infiltrator-Projects/Filesystem-Btrfs) | V2 | Namespace, bounded feature subset | Namespace, bounded subset | v1.0.12 | Hosted and freestanding real-image tests green | Additional Btrfs feature variants; future V2 BOOT |
| [Filesystem-exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT) | V2 | Namespace | Namespace | v0.1.17 | V2 hosted/firmware, read/write/corruption CI green | Future V2 BOOT consumer |
| [Filesystem-EXT2](https://github.com/Infiltrator-Projects/Filesystem-EXT2) | V2 | Namespace; DEFRAG_MAP | Namespace read-only | v1.0.16 | Native media and freestanding source CI green | V2 DEFRAG mutation/recovery; future V2 BOOT |
| [Filesystem-EXT3](https://github.com/Infiltrator-Projects/Filesystem-EXT3) | V2 | Namespace; DEFRAG_MAP | Namespace read-only | v1.0.15 | Native and package CI green | Journal variants, V2 DEFRAG mutation/recovery; future V2 BOOT |
| [Filesystem-EXT4](https://github.com/Infiltrator-Projects/Filesystem-EXT4) | V2 | Namespace | Namespace read-only | v1.0.19 | Canonical core, real images, package CI green | Future V2 BOOT; legacy DEFRAG migration |
| [Filesystem-FAT12](https://github.com/Infiltrator-Projects/Filesystem-FAT12) | V2 | Namespace | Namespace | v0.1.30 | Current main package CI green | Future V2 BOOT |
| [Filesystem-FAT16](https://github.com/Infiltrator-Projects/Filesystem-FAT16) | V2 | Namespace | Namespace | v0.1.22 | Current main package CI green | Future V2 BOOT |
| [Filesystem-FAT32](https://github.com/Infiltrator-Projects/Filesystem-FAT32) | V2 | Namespace | Namespace | v0.1.18 | Current main package CI green | Future V2 BOOT |
| [Filesystem-FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS) | V2 | Namespace | Namespace | v1.0.13 | Hosted and firmware CI green | Future V2 BOOT |
| [Filesystem-HFS](https://github.com/Infiltrator-Projects/Filesystem-HFS) | V2 | Namespace | Namespace | v1.0.13 | Real image and freestanding CI green | Future V2 BOOT |
| [Filesystem-HFSPlus](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus) | V2 | Namespace | Namespace | v1.0.12 | HFS+/HFSX hosted and freestanding CI green | Supported-feature coverage; future V2 BOOT |
| [Filesystem-Minix](https://github.com/Infiltrator-Projects/Filesystem-Minix) | V2 | Namespace | Namespace | v1.0.16 | Independent variant and firmware CI green | Future V2 BOOT on supported MINIX versions |
| [Filesystem-NTFS](https://github.com/Infiltrator-Projects/Filesystem-NTFS) | V2 | Namespace via ntfs-3g | Namespace via native C reader | **v1.0.19** | Exact packaged firmware kernel/initramfs, MFT corruption, dirty-state and 4Kn tests green; build and publishing workflows green | Unify hosted/native source and qualify more NTFS features; future V2 BOOT |
| [Filesystem-OFS](https://github.com/Infiltrator-Projects/Filesystem-OFS) | V2 | Namespace | Namespace (direct profile) | v0.1.17 | V2 real boot files CI green | Migrate separate Defrag host API; future V2 BOOT |
| [Filesystem-PFS3](https://github.com/Infiltrator-Projects/Filesystem-PFS3) | V2 | Namespace, supported subset | Namespace, supported subset | v1.0.12 | Packaged and firmware CI green | Large index variants; future V2 BOOT |
| [Filesystem-SFS](https://github.com/Infiltrator-Projects/Filesystem-SFS) | V2 | Namespace | Namespace | v1.0.11 | Actual packaged firmware namespace CI green | Future V2 BOOT |
| [Filesystem-SFS2](https://github.com/Infiltrator-Projects/Filesystem-SFS2) | V2 | Namespace | Namespace | v0.1.8 | Actual packaged V2 read-only CI green | Future V2 BOOT |
| [Filesystem-Swap](https://github.com/Infiltrator-Projects/Filesystem-Swap) | V2 | PROBE + format, no file namespace | Not applicable | v1.0.12 | Swap V2 tests and packages green | Correctly exclude SWAP as boot volume, not a missing reader |
| [Filesystem-UFS1](https://github.com/Infiltrator-Projects/Filesystem-UFS1) | V2 | Namespace | Namespace | v1.0.13 | Freestanding and package CI green | Native feature variants; future V2 BOOT |
| [Filesystem-UFS2](https://github.com/Infiltrator-Projects/Filesystem-UFS2) | V2 | Namespace | Namespace | v1.0.20 | Freestanding and package CI green | Future V2 BOOT |
| [Filesystem-XFS](https://github.com/Infiltrator-Projects/Filesystem-XFS) | V2 | Namespace, bounded features | Namespace, bounded features | v1.0.14 | Both packaged readers, real boot files and corrupt images CI green | Additional XFS variants; future V2 BOOT |
| [Filesystem-ZFS](https://github.com/Infiltrator-Projects/Filesystem-ZFS) | V2 | PROBE only | PROBE only | v1.0.10 | Verified OpenZFS label, checksum and V2 probe CI green | Implement native dataset mount, ZAP directory traversal and real file reads |

## Changes committed and released in this qualification cycle

EXT4's canonical-core unit test was repaired without leaking V1/private symbols ([commit](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/7310afae5284365ebc7fd2e3df596e43ddaffb33)); release v1.0.19 and both build/release workflows passed. Newly qualified, published `.deb` and native `.run` assets are also available for APFS v1.0.13, Btrfs v1.0.12, exFAT v0.1.17, EXT2 v1.0.16, EXT3 v1.0.15, HFS v1.0.13, HFSPlus v1.0.12, Minix v1.0.16, PFS3 v1.0.12, SFS v1.0.11, SFS2 v0.1.8, Swap v1.0.12, UFS1 v1.0.13, UFS2 v1.0.20, and XFS v1.0.14. The new releases omit old `.ko` assets.

NTFS's current main source includes a real bounded freestanding Common V2 MFT, INDEX_ROOT/INDEX_ALLOCATION and data-stream reader in `native/ntfs_ro_v2.c` linked into the packaged firmware ELF. Real 512/4096-byte geometry, kernel and initramfs reads, MFT fixup and dirty-volume corruption checks run through the actual extracted package binary. [NTFS v1.0.19](https://github.com/Infiltrator-Projects/Filesystem-NTFS/releases/tag/v1.0.19) **was published after its final build, package and real-media workflows passed**, with a .deb, native .run and SHA256SUMS manifest. The hosted path still uses ntfs-3g; full source convergence is not yet complete. See [NTFS V2 status](https://github.com/Infiltrator-Projects/Filesystem-NTFS/blob/main/docs/COMMON_V2_BOOT_STATUS.md).

Defragmenter current main already contains a Common V2 discovery and execution consumer. A correctness repair in [commit d4fcce82](https://github.com/Infiltrator-Projects/Defragmenter/commit/d4fcce822dd543c4ead30e26e36ea43f37209e5b) now requires `IFS_FS_END` before claiming an extent map complete, with a deliberately broken-page regression fixture. **Defragmenter's EXT worker still invokes retired V1 runtime services** for operations lacking complete provider-owned V2 DEFRAG PLAN/EXECUTE/VERIFY/RECOVER; it is not yet a V2-only application. Those native mutation operations require real implementation, not a renamed adapter or deleted functionality.

## Release rule

Do not mark a module fully BOOT-qualified until its exact published `.so` has passed the unmodified future production Common V2 BOOT validator and real OVMF read-only mounting, directory listing, kernel/initramfs reads, corrupt-feature refusal and unchanged-image qualification. No user's machines or BOOT Git repository code were changed.

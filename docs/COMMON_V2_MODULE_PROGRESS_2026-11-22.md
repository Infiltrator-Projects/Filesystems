# Common V2 module progress — 22 November 2026

## Evidence and qualification rules

All **23** current filesystem main-branch source trees export the Common V2 entry point (infiltrator_filesystem_api_v2). This is a **source capability review**, not a certificate for released ELF payloads. For the full contract see [Libraries Common V2](https://github.com/Infiltrator-Projects/Libraries/blob/main/include/infiltratr/filesystem_module_v2.h). A green main Actions workflow does not demonstrate an end-to-end boot.

The VM validator column reproduces **owner-reported historical Zabbix validation**, not verification against a V2 BOOT consumer. BOOT itself is being migrated elsewhere and was not modified. Hosted/firmware columns record the capabilities advertised by current source. “Namespace” means source declares CORE PROBE, MOUNT, STAT, LIST and READ, with important documented feature limits. “CI” is the number of inspected latest-revision workflows reporting success, **not** kernel/initramfs-in-OVMF proof. “Release” is the latest immutable GitHub release tag, which may predate the main-branch implementation.

## Provider matrix

| Repository | ABI on main | Hosted V2 source | Firmware V2 source | VM validator (reported) | CI successful | Release | Remaining blockers |
|---|---|---|---|---|---|---|---|
| [Filesystem-APFS](https://github.com/Infiltrator-Projects/Filesystem-APFS) | Common V2 | Namespace (flat-tree subset) | Namespace (flat-tree subset) | FAIL | 2/2 | v1.0.12 | Multilevel APFS; old released V1/.ko; actual V2 BOOT/OVMF |
| [Filesystem-Btrfs](https://github.com/Infiltrator-Projects/Filesystem-Btrfs) | Common V2 | Namespace (subset) | Namespace (subset) | FAIL | 2/2 | v1.0.11 | Old release .ko; qualifying current feature subset in V2 BOOT/OVMF |
| [Filesystem-exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT) | Common V2 | Namespace | Namespace | FAIL | 3/3 | v0.1.16 | Newer source vs released carrier; V2 BOOT/OVMF |
| [Filesystem-EXT2](https://github.com/Infiltrator-Projects/Filesystem-EXT2) | Common V2 | Namespace; DEFRAG map only | Namespace | FAIL | 4/4 | v1.0.15 | DEFRAG mutation missing; V2 BOOT/OVMF |
| [Filesystem-EXT3](https://github.com/Infiltrator-Projects/Filesystem-EXT3) | Common V2 | Namespace; DEFRAG map only | Namespace | FAIL | 2/2 | v1.0.14 | Journal variants; DEFRAG mutation; V2 BOOT/OVMF |
| [Filesystem-EXT4](https://github.com/Infiltrator-Projects/Filesystem-EXT4) | Common V2 | Namespace | Namespace | PASS | 2/2 | v1.0.19 | New V2-only fix released; V2 BOOT/OVMF still untested |
| [Filesystem-FAT12](https://github.com/Infiltrator-Projects/Filesystem-FAT12) | Common V2 | Namespace | Namespace | PASS | 1/1 | v0.1.30 | V2 BOOT/OVMF |
| [Filesystem-FAT16](https://github.com/Infiltrator-Projects/Filesystem-FAT16) | Common V2 | Namespace | Namespace | PASS | 1/1 | v0.1.22 | V2 BOOT/OVMF |
| [Filesystem-FAT32](https://github.com/Infiltrator-Projects/Filesystem-FAT32) | Common V2 | Namespace | Namespace | PASS | 1/1 | v0.1.18 | V2 BOOT/OVMF |
| [Filesystem-FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS) | Common V2 | Namespace | Namespace | FAIL | 4/4 | v1.0.13 | Released carrier and V2 BOOT/OVMF |
| [Filesystem-HFS](https://github.com/Infiltrator-Projects/Filesystem-HFS) | Common V2 | Namespace | Namespace | FAIL | 3/3 | v1.0.12 | Released carrier and V2 BOOT/OVMF |
| [Filesystem-HFSPlus](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus) | Common V2 | Namespace | Namespace | FAIL | 3/3 | v1.0.11 | HFS+/HFSX feature subsets; V2 BOOT/OVMF |
| [Filesystem-Minix](https://github.com/Infiltrator-Projects/Filesystem-Minix) | Common V2 | Namespace | Namespace | FAIL | 4/4 | v1.0.15 | Native format variants; V2 BOOT/OVMF |
| [Filesystem-NTFS](https://github.com/Infiltrator-Projects/Filesystem-NTFS) | Common V2 | Namespace (NTFS-3G) | **PROBE only** | FAIL | 2/2 | v1.0.18 | **Portable MFT/index/file reader absent in firmware**; old released V1/.ko |
| [Filesystem-OFS](https://github.com/Infiltrator-Projects/Filesystem-OFS) | Common V2 | Namespace | Namespace | PASS | 1/1 | v0.1.17 | V2 BOOT/OVMF; separate Defragmenter ABI integration |
| [Filesystem-PFS3](https://github.com/Infiltrator-Projects/Filesystem-PFS3) | Common V2 | Namespace (subset) | Namespace (subset) | FAIL | 3/3 | v1.0.11 | Index/large volume features; V2 BOOT/OVMF |
| [Filesystem-SFS](https://github.com/Infiltrator-Projects/Filesystem-SFS) | Common V2 | Namespace | Namespace | FAIL | 2/2 | v1.0.10 | Old released .ko; V2 BOOT/OVMF |
| [Filesystem-SFS2](https://github.com/Infiltrator-Projects/Filesystem-SFS2) | Common V2 | Namespace | Namespace | FAIL | 1/1 | v0.1.7 | Old released .ko; V2 BOOT/OVMF |
| [Filesystem-Swap](https://github.com/Infiltrator-Projects/Filesystem-Swap) | Common V2 | PROBE and FORMAT; no files | **N/A** | FAIL (expected) | 2/2 | v1.0.11 | Explicit nonbootable exclusion, not file namespace |
| [Filesystem-UFS1](https://github.com/Infiltrator-Projects/Filesystem-UFS1) | Common V2 | Namespace | Namespace | FAIL | 3/3 | v1.0.12 | UFS variant fixtures; V2 BOOT/OVMF |
| [Filesystem-UFS2](https://github.com/Infiltrator-Projects/Filesystem-UFS2) | Common V2 | Namespace | Namespace | FAIL | 3/3 | v1.0.19 | Released carrier; V2 BOOT/OVMF |
| [Filesystem-XFS](https://github.com/Infiltrator-Projects/Filesystem-XFS) | Common V2 | Namespace (subset) | Namespace (subset) | FAIL | 2/2 | v1.0.13 | Fragmented B-tree real media; V2 BOOT/OVMF |
| [Filesystem-ZFS](https://github.com/Infiltrator-Projects/Filesystem-ZFS) | Common V2 | **PROBE only** | **PROBE only** | FAIL | 2/2 | v1.0.10 | **Dataset mount, directory traversal and file reads absent** |

## Implementation verified this checkpoint

EXT4 main commit [7310afae](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/7310afae5284365ebc7fd2e3df596e43ddaffb33) fixes a real link regression: the canonical-core unit test tried to import private symbols from the V2-only public ELF. It now tests the **identical canonical source** also compiled into EXT4.so, preserving the production V2-only export map. Both build/test and native/portable packaging CI passed. Commit [8e663c5d](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/8e663c5d5a87fd466c178911ec2725a7fc11b421) then bumped the release to [v1.0.19](https://github.com/Infiltrator-Projects/Filesystem-EXT4/releases/tag/v1.0.19), containing the .deb and native .run. Both latest main Actions workflows finished successfully.

## Defragmenter ABI2 consumer blocker

The Defragmenter repository has Common V2 discovery/operations in defragger/native/filesystem_module_host.c, **but defragger/native/ext_module_loader.c still calls infiltrator_filesystem_ext_v1 and ifs_defrag_services_v1() for its regular operations**. A number of filesystem V2 DEFRAG groups advertise only MAP, not PLAN/EXECUTE/VERIFY/RECOVER. Removing those working legacy operations outright would break existing functionality. Consequently Defragmenter is **not yet V2-only**.

## Release and BOOT acceptance

No filesystem is marked fully BOOT-accepted without the **unchanged production V2 consumer** authenticating and executing its **actual release binary** in a disposable OVMF/UEFI environment, including real read-only mount, kernel and initramfs reads, directory traversal, negative/corrupt metadata and unchanged image tests. SWAP is inherently nonbootable. No claim is made of machine-level BOOT, Zabbix, ESP, NVRAM or Secure Boot changes. No BOOT repository code has been touched.

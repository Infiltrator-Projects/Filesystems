# Common V2 filesystem qualification

## Scope and evidence

This checkpoint covers all 23 independent repositories. The ABI authority is [Libraries at d23a526](https://github.com/Infiltrator-Projects/Libraries/blob/d23a52693fd3212368d327e1c1302e07ae9510ec/include/infiltratr/filesystem_module_v2.h). Each released AMD64 Debian asset was checked against its GitHub SHA-256 digest, extracted, and opened through `infiltrator_filesystem_api_v2` on an isolated test host. All 23 descriptors passed the header's `ifs_fs_api_v2_valid` check. The table records separate inspections of the actual exported symbols, the exact embedded ELF where present, and locally built providers.

**The rebuilt BOOT validator result is unavailable for every module.** The rebuilt v0.1.161 package has no `validate-module` executable. Its source at c98b1ca documents that it currently uses EFI filesystem drivers and lacks an authenticated Common V2 module loader. There are zero released modules with a verified kernel and initramfs read through that consumer. SWAP has no file namespace and its applicable consumer check is deliberate exclusion as a boot volume. [BOOT validation contract](https://github.com/Infiltrator-Projects/Boot/blob/c98b1ca4622228b5e7a3ae5e3205eca0e0282f7e/docs/VALIDATION.md#common-v2-qualification-gap) · [Rebuilt BOOT release](https://github.com/Infiltrator-Projects/Boot/releases/tag/v0.1.161)

A **namespace** descriptor advertises probe, read-only mount/unmount, stat, directory enumeration and read. **Embedded** means the carrier contains a second ELF with a freestanding/firmware descriptor. **Direct** means the installed ELF itself declares those profiles. **FS** means it declares freestanding only alongside hosted. A right arrow shows a change from the published binary to the audited main source. Additional symbols get an explicit V1 flag when they export a retired filesystem entry point. Tests are the provider's normal source test suite or portable standalone checks. Action links identify the exact build revision. “Blocked” in the validator column records that the executable could not be invoked.

| Repository and audited main | Released Debian version | ABI on release | Hosted CORE | Firmware carrier | Rebuilt validator | Source tests / Actions | Remaining work |
| --- | --- | --- | --- | --- | --- | --- | --- |
| [APFS](https://github.com/Infiltrator-Projects/Filesystem-APFS/tree/362c77a14bcb269d45ae0bf74082b2dbc94c66c3) | [1.0.12](https://github.com/Infiltrator-Projects/Filesystem-APFS/releases/tag/v1.0.12) | V2 + V1 | Probe, mount | None | Blocked | 5 passed. [2/2 green](https://github.com/Infiltrator-Projects/Filesystem-APFS/actions?query=sha%3A362c77a14bcb269d45ae0bf74082b2dbc94c66c3) | Add APFS catalogue, path and extent reads and freestanding support. Release also ships V1 and a kernel module. |
| [Btrfs](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/tree/9e7697705187503e2751bb016958e794a769c146) | [1.0.11](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/releases/tag/v1.0.11) | V2 | Probe, mount → Namespace | None → Embedded | Blocked | 5 passed, real boot files. [2/2 green](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/actions?query=sha%3A9e7697705187503e2751bb016958e794a769c146) | Publish the shared CRC32C SINGLE/DUP reader after consumer qualification. Released package includes a kernel module. |
| [exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT/tree/bd159b028644ec0df11de9aaf3476e1cfe98fec0) | [0.1.16](https://github.com/Infiltrator-Projects/Filesystem-exFAT/releases/tag/v0.1.16) | V2 | Namespace | Embedded | Blocked | 1 passed. [2/5 green](https://github.com/Infiltrator-Projects/Filesystem-exFAT/actions?query=sha%3Abd159b028644ec0df11de9aaf3476e1cfe98fec0) | Current firmware advertises read-only CORE. Qualify that release candidate through rebuilt BOOT. |
| [EXT2](https://github.com/Infiltrator-Projects/Filesystem-EXT2/tree/1457a3683523cb8636f4832dc321d90192be1a0f) | [1.0.15](https://github.com/Infiltrator-Projects/Filesystem-EXT2/releases/tag/v1.0.15) | V2 | Namespace | Embedded | Blocked | 6 passed. [1/4 green](https://github.com/Infiltrator-Projects/Filesystem-EXT2/actions?query=sha%3A1457a3683523cb8636f4832dc321d90192be1a0f) | Run the released carrier through the rebuilt consumer on valid and corrupt media. |
| [EXT3](https://github.com/Infiltrator-Projects/Filesystem-EXT3/tree/aba328372210e5f14636835b0f4893a93947e587) | [1.0.14](https://github.com/Infiltrator-Projects/Filesystem-EXT3/releases/tag/v1.0.14) | V2 | Namespace | Embedded | Blocked | 9 passed, also with PGO. [1/3 green](https://github.com/Infiltrator-Projects/Filesystem-EXT3/actions?query=sha%3Aaba328372210e5f14636835b0f4893a93947e587) | Release export cleanup and verify journal refusal plus kernel/initramfs through the rebuilt consumer. |
| [EXT4](https://github.com/Infiltrator-Projects/Filesystem-EXT4/tree/47814cf964ff26561b4566d04fe7d72f721c8f3d) | [1.0.17](https://github.com/Infiltrator-Projects/Filesystem-EXT4/releases/tag/v1.0.17) | V2 | Namespace | Embedded | Blocked | 8 passed. [2/2 green](https://github.com/Infiltrator-Projects/Filesystem-EXT4/actions?query=sha%3A47814cf964ff26561b4566d04fe7d72f721c8f3d) | Qualify this release through rebuilt BOOT on clean and corrupt EXT4. |
| [FAT12](https://github.com/Infiltrator-Projects/Filesystem-FAT12/tree/ba9abe495168c6d93b690af25377ae0ff533a03d) | [0.1.29](https://github.com/Infiltrator-Projects/Filesystem-FAT12/releases/tag/v0.1.29) | V2 | Namespace | Direct | Blocked | Portable passed. [1/1 green](https://github.com/Infiltrator-Projects/Filesystem-FAT12/actions?query=sha%3Aba9abe495168c6d93b690af25377ae0ff533a03d) | Qualify this release through rebuilt BOOT using the FAT12 volume and payloads. |
| [FAT16](https://github.com/Infiltrator-Projects/Filesystem-FAT16/tree/bcbacc59ee429a79834805282b7999fccb22a1e1) | [0.1.21](https://github.com/Infiltrator-Projects/Filesystem-FAT16/releases/tag/v0.1.21) | V2 | Namespace | Direct | Blocked | Portable passed. [1/1 green](https://github.com/Infiltrator-Projects/Filesystem-FAT16/actions?query=sha%3Abcbacc59ee429a79834805282b7999fccb22a1e1) | Qualify this release through rebuilt BOOT using the FAT16 volume and payloads. |
| [FAT32](https://github.com/Infiltrator-Projects/Filesystem-FAT32/tree/9b0981ed944dca74fea293d49af7c4b97176be50) | [0.1.17](https://github.com/Infiltrator-Projects/Filesystem-FAT32/releases/tag/v0.1.17) | V2 | Namespace | Direct | Blocked | Portable passed. [1/1 green](https://github.com/Infiltrator-Projects/Filesystem-FAT32/actions?query=sha%3A9b0981ed944dca74fea293d49af7c4b97176be50) | Qualify this release through rebuilt BOOT using the FAT32 volume and payloads. |
| [FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS/tree/797ca0b403ac724e59ce2e142458b36ef18299d9) | [1.0.12](https://github.com/Infiltrator-Projects/Filesystem-FFS/releases/tag/v1.0.12) | V2 | Namespace | Embedded | Blocked | 5 passed. [4/4 green](https://github.com/Infiltrator-Projects/Filesystem-FFS/actions?query=sha%3A797ca0b403ac724e59ce2e142458b36ef18299d9) | Qualify the released Amiga FFS reader and its actual payloads through rebuilt BOOT. |
| [HFS](https://github.com/Infiltrator-Projects/Filesystem-HFS/tree/f20e40f02ea45d44464d49e43a0d1a7ec630a398) | [1.0.12](https://github.com/Infiltrator-Projects/Filesystem-HFS/releases/tag/v1.0.12) | V2 | Namespace | Embedded | Blocked | 2 passed. [2/3 green](https://github.com/Infiltrator-Projects/Filesystem-HFS/actions?query=sha%3Af20e40f02ea45d44464d49e43a0d1a7ec630a398) | Qualify classic HFS catalogue and extent handling through the rebuilt consumer. |
| [HFSPLUS](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus/tree/6ab0775e35cc5249e493ce31ed074c44fb1f48a9) | [1.0.11](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus/releases/tag/v1.0.11) | V2 | Namespace | Embedded | Blocked | 2 passed. [2/3 green](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus/actions?query=sha%3A6ab0775e35cc5249e493ce31ed074c44fb1f48a9) | Qualify HFS+/HFSX catalogue and extent coverage through the rebuilt consumer. |
| [MINIX](https://github.com/Infiltrator-Projects/Filesystem-Minix/tree/83dc93992791a35ca4c5030cfc589d3e9e786d44) | [1.0.15](https://github.com/Infiltrator-Projects/Filesystem-Minix/releases/tag/v1.0.15) | V2 | Namespace | Embedded | Blocked | 3 passed. [4/4 green](https://github.com/Infiltrator-Projects/Filesystem-Minix/actions?query=sha%3A83dc93992791a35ca4c5030cfc589d3e9e786d44) | Qualify each supported MINIX variant on the released firmware reader. |
| [NTFS](https://github.com/Infiltrator-Projects/Filesystem-NTFS/tree/324ee0bb0294ca4ce207c18add5f9c4e6b44757e) | [1.0.18](https://github.com/Infiltrator-Projects/Filesystem-NTFS/releases/tag/v1.0.18) | V2 + V1 | Probe, mount → Namespace | None | Blocked | 8 passed. [2/2 green](https://github.com/Infiltrator-Projects/Filesystem-NTFS/actions?query=sha%3A324ee0bb0294ca4ce207c18add5f9c4e6b44757e) | Main provides hosted MFT reads with ntfs-3g. Implement a shared freestanding namespace reader. Release includes V1 and a kernel module. |
| [OFS](https://github.com/Infiltrator-Projects/Filesystem-OFS/tree/58e9e2e750e1f84591f025709338df5e3c29f099) | [0.1.17](https://github.com/Infiltrator-Projects/Filesystem-OFS/releases/tag/v0.1.17) | V2 | Namespace | FS → Direct | Blocked | make check, real boot files. [1/1 green](https://github.com/Infiltrator-Projects/Filesystem-OFS/actions?query=sha%3A58e9e2e750e1f84591f025709338df5e3c29f099) | Release the root-sector error fix, explicit firmware profile and source-package cleanup after consumer qualification. |
| [PFS3](https://github.com/Infiltrator-Projects/Filesystem-PFS3/tree/693e5a44fe890aee73d68f9bec4f2b81d48a36fb) | [1.0.11](https://github.com/Infiltrator-Projects/Filesystem-PFS3/releases/tag/v1.0.11) | V2 | Namespace | Embedded | Blocked | 4 passed. [2/3 green](https://github.com/Infiltrator-Projects/Filesystem-PFS3/actions?query=sha%3A693e5a44fe890aee73d68f9bec4f2b81d48a36fb) | Qualify the documented direct-index subset through the rebuilt consumer. Check large-sector and superindex coverage separately. |
| [SFS](https://github.com/Infiltrator-Projects/Filesystem-SFS/tree/7842b64379887b71ae0e70bf0e122cf79e233d64) | [1.0.10](https://github.com/Infiltrator-Projects/Filesystem-SFS/releases/tag/v1.0.10) | V2 | Probe, mount → Namespace | None → Embedded | Blocked | 5 passed. [2/2 green](https://github.com/Infiltrator-Projects/Filesystem-SFS/actions?query=sha%3A7842b64379887b71ae0e70bf0e122cf79e233d64) | Publish embedded object/B-tree reader after consumer qualification. Released package includes a kernel module. |
| [SFS2](https://github.com/Infiltrator-Projects/Filesystem-SFS2/tree/86e04e9b79e685686cd68d6c807b5e61fd1c109e) | [0.1.7](https://github.com/Infiltrator-Projects/Filesystem-SFS2/releases/tag/v0.1.7) | V2 | Probe, mount → Namespace | None → Embedded | Blocked | 4 passed. [1/1 green](https://github.com/Infiltrator-Projects/Filesystem-SFS2/actions?query=sha%3A86e04e9b79e685686cd68d6c807b5e61fd1c109e) | Publish embedded native namespace after consumer qualification. Released package includes a kernel module. |
| [SWAP](https://github.com/Infiltrator-Projects/Filesystem-Swap/tree/9eb861e1e51ca86e91486ea02956d8f73eee3a40) | [1.0.11](https://github.com/Infiltrator-Projects/Filesystem-Swap/releases/tag/v1.0.11) | V2 | Probe and format | Inapplicable | Blocked | 2 passed. [0/2 green](https://github.com/Infiltrator-Projects/Filesystem-Swap/actions?query=sha%3A9eb861e1e51ca86e91486ea02956d8f73eee3a40) | Nonbootable. Qualify updated V2 sector reads and confirm explicit exclusion by rebuilt BOOT. |
| [UFS1](https://github.com/Infiltrator-Projects/Filesystem-UFS1/tree/8c7133c03baf8205177f8a1ece19d980ebb68d20) | [1.0.12](https://github.com/Infiltrator-Projects/Filesystem-UFS1/releases/tag/v1.0.12) | V2 | Namespace | Embedded | Blocked | 2 passed. [1/3 green](https://github.com/Infiltrator-Projects/Filesystem-UFS1/actions?query=sha%3A8c7133c03baf8205177f8a1ece19d980ebb68d20) | Qualify UFS1 inode, indirect-block and file reads through the rebuilt consumer. |
| [UFS2](https://github.com/Infiltrator-Projects/Filesystem-UFS2/tree/72a7d0f613e80de8d851aad354f6aa2e60bd7ae3) | [1.0.19](https://github.com/Infiltrator-Projects/Filesystem-UFS2/releases/tag/v1.0.19) | V2 | Namespace | Embedded | Blocked | 2 passed. [1/3 green](https://github.com/Infiltrator-Projects/Filesystem-UFS2/actions?query=sha%3A72a7d0f613e80de8d851aad354f6aa2e60bd7ae3) | Qualify UFS2 inode, indirect-block and file reads through the rebuilt consumer. |
| [XFS](https://github.com/Infiltrator-Projects/Filesystem-XFS/tree/acbbec2a45a993c393d9a72f189dec64f0cc55b1) | [1.0.13](https://github.com/Infiltrator-Projects/Filesystem-XFS/releases/tag/v1.0.13) | V2 | Probe, mount | None | Blocked | 5 passed. [2/2 green](https://github.com/Infiltrator-Projects/Filesystem-XFS/actions?query=sha%3Aacbbec2a45a993c393d9a72f189dec64f0cc55b1) | Implement allocation-group inode, extent and directory reads and freestanding execution. |
| [ZFS](https://github.com/Infiltrator-Projects/Filesystem-ZFS/tree/fc09e868f05e7ad78a55ba8b03f21af4d13e9fba) | [1.0.10](https://github.com/Infiltrator-Projects/Filesystem-ZFS/releases/tag/v1.0.10) | V2 + V1 | Probe | None | Blocked | 4 passed. [2/2 green](https://github.com/Infiltrator-Projects/Filesystem-ZFS/actions?query=sha%3Afc09e868f05e7ad78a55ba8b03f21af4d13e9fba) | Implement a supported pool/dataset namespace, verified block traversal and freestanding reads. Released binary exports V1. |

## Binary and media findings

The Debian packages for APFS, Btrfs, NTFS, SFS and SFS2 contain actual filesystem-specific kernel modules. Each of the five releases also advertises a standalone kernel module asset. APFS, NTFS and ZFS additionally export their format-specific V1 entry point. The remaining released ELF files did not export a V1 filesystem entry point. Across the candidate source trees the package checkers now require the exact installed filesystem module and manual.

Btrfs exercised both the hosted carrier and its extracted, byte-identical no-import ELF on an independent `mkfs.btrfs` image. Both returned the original Debian kernel and compressed initramfs byte-for-byte on 512-byte and 4096-byte hosts. Independent corruption of both metadata replicas and of file data returned a corruption error. An injected host read failure returned an I/O error. Complete image hashes matched before and after testing. The supported subset includes CRC32C, uncompressed file extents, and single-device SINGLE/DUP profiles. [Btrfs reader](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/commit/4ec22441c06562b9ded17d3020cd27d67c3d0fdd)

OFS read the kernel and compressed initramfs from a disposable image produced by amitools on 512-byte and 4096-byte hosts. Changing another byte in the same OFS data block caused a one-byte read to fail its checksum. Both media hashes stayed unchanged. A separate test distinguishes a root-sector I/O error from a damaged root checksum. The standalone V2 defragmentation export remains present. [OFS reader and packaging](https://github.com/Infiltrator-Projects/Filesystem-OFS/commit/58e9e2e750e1f84591f025709338df5e3c29f099)

## Earlier engineering records

The following entries are retained as development history. Their release versions, consumer binaries and results predate the current checkpoint above.

## Direct source repairs made at this checkpoint

- [SFS2 `e5cca1bf`](https://github.com/Infiltrator-Projects/Filesystem-SFS2/commit/e5cca1bf37273f4e2500f887abaf44a33b648460): Native Common V2 mount refuses checksum-valid bitmap marking required live metadata free. Six deliberate corrupt-media cases added to the actual V2 mounted interface. Exact-head hosted and DEB/RUN workflows passed. No firmware release published.
- [SFS `9249561f`](https://github.com/Infiltrator-Projects/Filesystem-SFS/commit/9249561f0a0ebe88acf8ef78d8991ef343911531): Same invariant qualified independently for classic SFS0 metadata/layout with eight checksum-valid corruption cases through the production V2 API. Exact-head hosted source and DEB/native RUN package workflows both passed. No firmware release published.

- [APFS `1ddc6b37`](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/1ddc6b37f1a85831f480971202da874062f90f6e): The actual hosted Common V2 mount now preserves a failed checkpoint read as `IFS_FS_IO_ERROR`, rather than falsely reporting disk corruption. Native code still enforces genuine APFS checkpoint/volume checks. A regression injects an I/O failure after a checksum-valid superblock probe, through CORE.MOUNT. Exact-commit hosted source and DEB/RUN workflows both passed. Full APFS namespace, firmware ELF and BOOT compatibility remain unimplemented.

## EXT4 continuation — 21 November 2026

The independent EXT4 provider now returns `IFS_FS_NO_SPACE` for small
STAT/LIST/label output buffers and preserves true Common V2 host I/O
failure codes in tests. The actual freestanding ELF exports the V2 API,
has no hosted imports, is byte-identical to `.infiltrator_boot` inside the
production SO, and is exercised through V2 against an independently
formatted EXT4 namespace with 512-byte/4Kn I/O, a checksum-protected
indexed HTree directory, real file payloads, and corrupted metadata.
New commits: [b75739d0](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/b75739d08b1e37df05c9e098cb7e126128f53deb),
[0d01cdfe](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/0d01cdfedf0be946a8141423fbf42ef4f4f0bec4),
[99fb2265](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/99fb22657822c13be1072ba9cc1d6d3267410f22),
and [a99dbb40](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/a99dbb40e73b0dac25085870c3a5541c988b792c).

The previously dead BOOT references in both EXT4 CI workflows have
been removed; CI now verifies the genuine published BOOT release
DEB checksum and demands its production validator/EFI loader before
any OVMF qualification or release publication. The real v0.1.119
package download succeeds but **does not ship validate-module**;
therefore BOOT qualification remains blocked by that external
contract. The hosted CI suite passes. The portable and CPU-native RUN
packages also pass their build/functional checks, but the release job
does not publish because the BOOT gate cannot run. Existing v1.0.16
release assets remain unchanged. No fake validator, V1 fallback, kernel
filesystem adapter or live-device modification was introduced.

## Current external BOOT validator dependency

Current GitHub Actions logs for SWAP, PFS3 and HFSPLUS show that their
downloaded BOOT 0.1.77 DEB **does not contain**
`/usr/lib/infiltrator-boot/validate-module`. The SWAP source tests
and the PFS3/HFSPLUS freestanding tests can pass independently while
the mandatory BOOT test fails before invoking a validator. EXT2's
current BOOT 0.1.72 artifact has a SHA-256 mismatch against the workflow
pin. The current BOOT `main` tree also lacks the previously documented standalone `packaging/validate-module.c` and `boot/so_elf.c` source paths. This is an unresolved production-validator delivery/contract dependency to be reconciled separately from filesystem code. Do not replace either gate with a fake validator, weaken checks,
or declare firmware readiness. Reconcile the immutable validator
and release artifact contract with the separately maintained BOOT
repository and then run all relevant exact-release qualification.

## Published artifacts vs current source tree

The latest published releases for APFS, Btrfs, HFSPLUS, NTFS, PFS3,
SFS2, SFS, FAT16, FAT32 and OFS still have independent `.ko` assets.
Those historical assets are **not** Common V2 substitutes and are not
evidence of acceptance. The newer `.so`-only build workflows in some
repositories do not retroactively repair an old GitHub release.

## Acceptance rule

Only mark a boot-capable format complete after its exact released `.so`
exports only Common V2, contains a verifiable no-import firmware reader,
passes the **unchanged** BOOT production validator, reads real kernel and
initramfs bytes in disposable OVMF, rejects corrupt/unsupported media,
proves no source-image mutation, and ships tested Debian/native RUN packages
with passing CI on the same `main` revision. SWAP is an explicit
nonbootable exception: its V2 probe/format must work and BOOT must reject it.

No machine configuration, ESP, Secure Boot keys, GRUB fallback or firmware
boot order was altered in this checkpoint.

## Independent 23-repository checkpoint — 21 November 2026, later source revisions

These rows track independent repositories and their current published version; an existing release can predate the new `main` fixes. The validator column is **the user's Zabbix baseline, never a rerun**. `PASS` here does **not** mean acceptance under the stricter release/OVMF criteria. Every production ABI listed is Common V2; historical test-only V1 source and old `.ko` release assets must not be mistaken for runtime interfaces.

| Module / independent repository | Latest published tag | Hosted Common V2 | Firmware source | Zabbix validator baseline | Current test/packaging evidence | Remaining production blocker |
|---|---|---|---|---|---|---|
| [FAT12](https://github.com/Infiltrator-Projects/Filesystem-FAT12) | v0.1.29 | V2 namespace | Working per baseline | PASS | Published DEB/RUN; source/release CI green | Recheck exact release with validator and OVMF |
| [FAT16](https://github.com/Infiltrator-Projects/Filesystem-FAT16) | v0.1.21 | V2 namespace | Working per baseline | PASS | Published DEB/RUN; source/release CI green | Recheck exact release with validator and OVMF |
| [FAT32](https://github.com/Infiltrator-Projects/Filesystem-FAT32) | v0.1.17 | V2 namespace | Working per baseline | PASS | Published DEB/RUN; source/release CI green | Recheck exact release with validator and OVMF |
| [OFS](https://github.com/Infiltrator-Projects/Filesystem-OFS) | v0.1.17 | V2 namespace | Working per baseline | PASS | Published DEB/RUN; source/release CI green | Recheck exact release with validator and OVMF |
| [EXT4](https://github.com/Infiltrator-Projects/Filesystem-EXT4) | v1.0.17 | V2 namespace + error-safe reads | Embedded freestanding reader | PASS | Published DEB/RUN; hosted+package CI green | Exact production validator unavailable in BOOT package; OVMF unqualified |
| [APFS](https://github.com/Infiltrator-Projects/Filesystem-APFS) | v1.0.12 | V2 probe/mount/query; strict NX/APSB object identity and unsupported-feature refusal | No firmware reader | FAIL | Production real-media V2/ASan and DEB/RUN CI green (8fc4c83c) | Native directory/path/extent reads, freestanding ELF, BOOT validator and OVMF |
| [Btrfs](https://github.com/Infiltrator-Projects/Filesystem-Btrfs) | v1.0.11 | V2 probe/mount/query; new feature/geometry checks | No firmware reader | FAIL | Hosted+package CI green (6616b211) | Chunk/root trees, namespace, ELF, BOOT and OVMF |
| [exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT) | v0.1.16 | V2 namespace | Embedded freestanding ELF | FAIL | Hosted CI green; firmware/BOOT gates red | Released validator, OVMF/corrupt-media qualification |
| [EXT2](https://github.com/Infiltrator-Projects/Filesystem-EXT2) | v1.0.15 | V2 namespace | Embedded freestanding ELF | FAIL | Hosted and Defragmenter CI green at e25ef963; DEB/RUN qualification green, publication gate blocked | Pinned BOOT package checksum mismatch; validator/OVMF |
| [EXT3](https://github.com/Infiltrator-Projects/Filesystem-EXT3) | v1.0.14 | V2 clean-volume namespace | Embedded freestanding ELF | FAIL | Hosted and DEB/RUN CI green at 6f89b189; BOOT gate red | Production validator/OVMF; fail closed on dirty journal |
| [FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS) | v1.0.12 | V2 namespace | Embedded freestanding ELF | FAIL | Hosted/freestanding/package CI green at cac31ca9; name output now atomic | Production BOOT validator and OVMF on exact release |
| [HFSPLUS](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus) | v1.0.11 | V2 catalog/extent namespace subset | Embedded freestanding ELF | FAIL | V2 UTF-16 converter fixed at 2c694c06; hosted/firmware/package CI green; immutable old tag blocks publish | Qualified format coverage; validator/OVMF |
| [HFS](https://github.com/Infiltrator-Projects/Filesystem-HFS) | v1.0.12 | V2 catalog/extent namespace subset | Embedded freestanding ELF | FAIL | Hosted/firmware/package CI green | MacRoman/overflow boundaries; validator/OVMF |
| [MINIX](https://github.com/Infiltrator-Projects/Filesystem-Minix) | v1.0.15 | V2 namespace | Embedded freestanding ELF | FAIL | Hosted, freestanding, DEB/RUN CI green at 0d3fb3ad | Real production BOOT validator and OVMF |
| [NTFS](https://github.com/Infiltrator-Projects/Filesystem-NTFS) | v1.0.18 | Hosted V2 MFT namespace with ntfs-3g | No no-import freestanding ELF | FAIL | Hosted+package CI green | Replace ntfs-3g runtime with native freestanding MFT/index/runlist reader |
| [PFS3](https://github.com/Infiltrator-Projects/Filesystem-PFS3) | v1.0.11 | V2 direct-index namespace subset | Embedded freestanding ELF | FAIL | Hosted/firmware/package CI green | Large-sector/superindex coverage; validator/OVMF |
| [SFS2](https://github.com/Infiltrator-Projects/Filesystem-SFS2) | v0.1.7 | V2 native namespace and corruption checks | Embedded freestanding ELF | FAIL | Native V2 name buffer safety at dc1eccc8; shared-library and DEB/RUN CI green | Production validator/OVMF; old release has KO |
| [SFS](https://github.com/Infiltrator-Projects/Filesystem-SFS) | v1.0.10 | V2 native namespace and corruption checks | Embedded freestanding ELF | FAIL | Native V2 name buffer safety at 5f204d80; hosted + DEB/RUN CI green | Production validator/OVMF; old release has KO |
| [SWAP](https://github.com/Infiltrator-Projects/Filesystem-Swap) | v1.0.11 | V2 probe/format, deliberately no namespace | Not applicable; nonbootable | FAIL expected | V2 tests/build pass; BOOT-rejection gate red | Production validator must prove intentional rejection |
| [UFS1](https://github.com/Infiltrator-Projects/Filesystem-UFS1) | v1.0.12 | V2 native inode namespace; updated SDK pin | Embedded freestanding ELF | FAIL | Hosted CI green; firmware/package BOOT gate red | Production BOOT validator/OVMF; unqualified release |
| [UFS2](https://github.com/Infiltrator-Projects/Filesystem-UFS2) | v1.0.19 | V2 native inode namespace; updated SDK pin | Embedded freestanding ELF | FAIL | Hosted CI green; firmware/package BOOT gate red | Production BOOT validator/OVMF; unqualified release |
| [XFS](https://github.com/Infiltrator-Projects/Filesystem-XFS) | v1.0.13 | V2 probe/mount/query; new feature checks | No firmware reader | FAIL | Hosted+package CI green (31f0addf) | Native AG/inode/directory/extents, ELF, validator/OVMF |
| [ZFS](https://github.com/Infiltrator-Projects/Filesystem-ZFS) | v1.0.10 | V2 probe; label I/O propagation; updated SDK pin | No firmware reader | FAIL | Hosted CI green; package after new pin pending | Native MOS/dataset/dnode/block-pointer namespace, ELF, BOOT/OVMF |

### Filesystem-repository main commits at this checkpoint

- Btrfs `6616b211ec7ae031d3f375e51cde74918d5daf7b`: fail closed for unsupported V2 on-disk features/geometry; source + DEB/RUN Actions both green.
- XFS `03415cb64d7f93187b235e50cd0ca5db07047b23` plus compiler correction `315cf438b38cbc8c240a07804dab541e8b22b690`: fail closed on incomplete/unsupported CRC-valid v5 volumes; `31f0addf9f61450009adaf4f7e12033e8b86c137` removes the premature hosted release job. New source and package Actions both green; no new version published.
- APFS `4fc3df182c1b2cd21d05b7dfd497eb8e8e276625`: Common V2 sector-aligned NXSB reads for >=8 KiB sectors, checksum failure/nonmutation regression; hosted and package Actions green.
- ZFS `1a78b5a5e575631c97e6d77463cbf03dce8b89d2`: propagate an I/O failure reading vdev label configuration after committed uberblock discovery; initial hosted/package Actions green. `bb83d5c37d491e5c8219801686df2d5e05a34644` updates the SDK pin to current Common V2; independent validation is separate.
- UFS1 `184f48cded86d4a6f63afc6241fee649e69d5214` and UFS2 `38f70872cf611fcfefe3a76cb66ef0db4575e5e6`: pin current Common V2 SDK; hosted CI green, existing firmware workflows still blocked on their unchanged BOOT validator step.

### External validator contract — do not fake or weaken

The available released BOOT DEB pinned in multiple independent firmware workflows (`0.1.77`) has no `/usr/lib/infiltrator-boot/validate-module`. EXT4 uses `0.1.119` with the same missing executable, and EXT2's pinned `0.1.72` DEB has a release checksum mismatch. These are concrete external release-contract blockers, **not evidence that a specific module is V1**. The live Zabbix module binaries and installed validator were not available in this GitHub-only task, so no new module was asserted to pass that validator. No BOOT, VM, ESP, Secure Boot, GRUB or firmware boot-order changes have been made.

### Acceptance policy

No module has been newly designated complete by this checkpoint. `main` source/package success is only the first qualification layer. Each boot-capable released SO still requires byte-for-byte packaged firmware payload verification, the production validator, real OVMF kernel+initramfs reads, valid/invalid media, nonmutation, and matching green CI/release. SWAP remains intentionally nonbootable.

### Source continuation — EXT2, EXT3 and Minix (same-day independent filesystem work)

No BOOT repository files, VM configuration, ESP, keys, GRUB or NVRAM settings were modified.

| Module | Main commits (production, regression) | Verified scope on exact test commit | Remaining acceptance blocker |
|---|---|---|---|
| EXT2 | [47209462](https://github.com/Infiltrator-Projects/Filesystem-EXT2/commit/472094627ef96dac904b3c11bc98bb46c827ce9c), [e25ef963](https://github.com/Infiltrator-Projects/Filesystem-EXT2/commit/e25ef963c9993d565372d3b0403658182728de9c) | Production CORE.STAT/CORE.LIST now return `IFS_FS_NO_SPACE` for insufficient name output space without changing caller buffers. Real V2 media regression and Defragmenter CI passed. DEB/RUN qualification passed, publication gate rejected because mandatory BOOT gate failed. | Pinned BOOT v0.1.72 package lacks executable validator; the source/hosted tests passed, but no qualified OVMF or new release. |
| EXT3 | [58a0aec8](https://github.com/Infiltrator-Projects/Filesystem-EXT3/commit/58a0aec847beb3a41f23eac1c50bee6534529348), [6f89b189](https://github.com/Infiltrator-Projects/Filesystem-EXT3/commit/6f89b189ad59a3a0e08b8b5b0d8dfc64a0eaef82) | Production V2 STAT/LIST buffer error correction and nonmutation regression using real mkfs.ext3 image. Hosted, source and DEB/RUN workflows passed. | BOOT v0.1.72 package does not expose required validator; no OVMF qualification or new version. |
| MINIX | [d4fb686d](https://github.com/Infiltrator-Projects/Filesystem-Minix/commit/d4fb686d5dc503dc3f743130d01cf2fed81a5b49), [0d3fb3ad](https://github.com/Infiltrator-Projects/Filesystem-Minix/commit/0d3fb3addcdd7310597011165d731b59d24bb505) | Production V2 STAT/LIST now distinguish short name buffers from malformed requests, verified without a partial write on existing variant disk fixtures. Exact-head hosted and freestanding CI passed. | Released package/production BOOT validation and OVMF still not qualified; no new version. |

These are real provider-side error-contract repairs, **not** claims of complete Common V2/BOOT acceptance. The installed Zabbix validator PASS/FAIL observations from the user's original 23-module baseline were not rerun here. Production validator failures were traced to the missing `validate-module` executable in the downloaded, checksum-verified BOOT packages; no alternative validator or bypass was introduced.

### Same-day Common V2 native reader and output contract continuation

The following repairs were committed to the independent filesystem repositories on `main`, without touching BOOT, installed media, firmware settings or Secure Boot keys. These correct real V2 reader behavior, and do **not** qualify released EFI bootability.

| Module | Production fix and exact regression commit | Exact-head qualification | Still required |
|---|---|---|---|
| FFS | [5e73c227](https://github.com/Infiltrator-Projects/Filesystem-FFS/commit/5e73c22777150e4f80a216c371eb6e250800ef50), [cac31ca9](https://github.com/Infiltrator-Projects/Filesystem-FFS/commit/cac31ca956b9dd2158de8d885ff7567a64ad56aa): native Amiga name conversion preflights full Latin-1→UTF-8 length, avoids partially written outputs; stat/list real BOOT-media test | Shared-object, strict carrier, freestanding and DEB/RUN workflows all green | Immutable exact release through production BOOT validator, OVMF kernel/initramfs and corruption checks |
| SFS | [c5329e70](https://github.com/Infiltrator-Projects/Filesystem-SFS/commit/c5329e70f5cda4e14209cc0bb223b43a29af72ec), [5f204d80](https://github.com/Infiltrator-Projects/Filesystem-SFS/commit/5f204d80d08aa00ced7b3a4e5e4b60f8221e6c15): correct short-buffer `NO_SPACE` for root and object names; no partial output; actual Common V2 stat/list regression | Hosted/core and DEB/RUN workflows green | Production validator, exact release and OVMF |
| SFS2 | [4565960b](https://github.com/Infiltrator-Projects/Filesystem-SFS2/commit/4565960b84efd9bd1cb2706aaa6a320dbf1520f1), [dc1eccc8](https://github.com/Infiltrator-Projects/Filesystem-SFS2/commit/dc1eccc88412010369341787dfb1c17cf36a438c): preflight complete UTF-8 object-name output and test untouched short stat/list buffers | Independent shared-library and DEB/RUN qualification green | Production validator, exact release and OVMF |
| HFSPLUS | [3275b249](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus/commit/3275b24995b4aa091ae44fc4c8601360499d817e), [2c694c06](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus/commit/2c694c06dd15a7b6ad948b1bf33e4f54b0c5dc71): preflight full UTF-16BE surrogate validity/UTF-8 length before writing, regression through V2 STAT/LIST | Hosted, freestanding and package qualification green; publishing blocked because v1.0.11 tag belongs to older commit | Complete HFS+ Unicode feature support, production validator, exact release and OVMF |

For Minix, exact-head source, native freestanding and DEB/RUN Actions are now green at `0d3fb3ad`; production BOOT/OVMF remains unqualified. Do not infer completion from a successful source build or package job.

The published-release inventory was rechecked against the 23 independent GitHub repositories on 21 November 2026; FAT12 v0.1.29, FAT16 v0.1.21 and FAT32 v0.1.17 supersede prior matrix versions. The original five Zabbix validator PASS results are the user's observations, not a rerun through live installed BOOT.

## Top-six focused follow-up — 21 November 2026

The requested first six are FAT12, FAT16, FAT32, OFS, EXT4 and APFS.
No BOOT repository, Zabbix VM, EFI system partition, firmware configuration
or live filesystem was changed.

| Module | Audited current `main` | Latest published tag | Follow-up result |
|---|---|---|---|
| FAT12 | `cc97ad5f` | `v0.1.29` | Existing V2 reader retained. CI green. Three commits after release remove non-shipped retired V1 defrag code; package/BOOT acceptance not rerun. |
| FAT16 | `7547009c` | `v0.1.21` | Existing V2 reader retained. CI green. Three post-release commits remove retired V1 source; no filesystem reader change identified. |
| FAT32 | `8a93b385` | `v0.1.17` | Existing V2 reader retained. CI green. Three post-release commits remove retired V1 source; no filesystem reader change identified. |
| OFS | `75739768` | `v0.1.17` | Existing V2 reader retained. CI green. Five post-release commits include mount/session and failed-flush correctness repairs; the published tag still predates these changes. |
| EXT4 | `7939eb92` | `v1.0.17` | Existing V2 source and freestanding payload retained. Hosted/package CI green and release head matches current source; production BOOT/OVMF gate still not independently available. |
| APFS | `8fc4c83c` tested source, with subsequent documentation commit | `v1.0.12` | Real production repairs: [c8c38cad](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/c8c38cada3a1da4de7af42e07d089b6fedf46aad) requires physical NX identity, active-checkpoint identity and APSB type, rejects every unsupported volume-incompatible bit; [de0b818b](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/de0b818b5e03ccc3450b80097772ff562874f7c5) returns V2 UNSUPPORTED separately from corruption/I/O; [8fc4c83c](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/8fc4c83c8f1c1526fe2259a1560aab5e34dfb3c9) qualifies production V2 on independent disposable APFS media with checksum-valid corrupt/unsupported variants. Hosted, AddressSanitizer and package CI passed; firmware reading is **not implemented**. |

These rows describe audited source and published artifact revisions separately.
The five original `PASS` values are the previously supplied Zabbix validator
observations. There has been no new production BOOT validation or OVMF
qualification. **APFS is not complete** and must not be advertised or released
as BOOT compatible. Release/version changes require their applicable
validation gates, not merely a successful host compilation.

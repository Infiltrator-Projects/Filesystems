# Common V2 / BOOT qualification ledger — 21 November 2026

This is the **current implementation checkpoint**, not a completion declaration.
Scope is exactly the 23 independent `Infiltrator-Projects/Filesystem-*`
repositories. Common V2 is defined exclusively by
`Libraries/include/infiltratr/filesystem_module_v2.h`.
A compiled V2 export or a successful hosted CI job is NOT evidence that
an unmodified BOOT validator accepted the **released** carrier or that OVMF
read a real EFI-stub kernel/initramfs. The original validator results below
are the user's Zabbix VM baseline, not a rerun against new binaries.
Historical published asset lists must be audited separately from current main.

| Module / repository | Source VERSION | Production ABI evidence | Hosted V2 | Freestanding/UEFI source state | Zabbix reported validator | Tests and remaining release blockers |
|---|---|---|---|---|---|---|
| [FAT12](https://github.com/Infiltrator-Projects/Filesystem-FAT12) | 0.1.27 | V2, historical defrag export still needs release audit | Namespace read | Local working (not rerun) | PASS | Latest release workflow failed; preserve current working module; remove V1 without losing defrag parity |
| [FAT16](https://github.com/Infiltrator-Projects/Filesystem-FAT16) | 0.1.19 | V2 source, published old kernel/EFI assets | Namespace read | Local working (not rerun) | PASS | Latest release workflow failed; validate package on current production BOOT |
| [FAT32](https://github.com/Infiltrator-Projects/Filesystem-FAT32) | 0.1.15 | V2 source, legacy tests/assets | Namespace read | Local working (not rerun) | PASS | Latest release workflow failed; validate package and eliminate .ko/.efi as substitutes |
| [OFS](https://github.com/Infiltrator-Projects/Filesystem-OFS) | 0.1.16 | V2 source; legacy published kernel asset | Namespace read | Local working (not rerun) | PASS | Latest release workflow failed; preserve qualified SO and remove non-V2 runtime |
| [EXT4](https://github.com/Infiltrator-Projects/Filesystem-EXT4) | 1.0.16 | Production V2; legacy code test-only | Probe/mount/stat/list/read | Embedded freestanding reader, firmware CI red | PASS | Exact-head package + production validator/OVMF requalification |
| [APFS](https://github.com/Infiltrator-Projects/Filesystem-APFS) | 1.0.12 | V2-only production; historical test code | Probe/mount/query only | No freestanding reader | FAIL | APFS host I/O error propagation fixed in 1ddc6b37; native catalog/path/extent reader, feature policy and OVMF still missing |
| [Btrfs](https://github.com/Infiltrator-Projects/Filesystem-Btrfs) | 1.0.11 | V2-only production; historical test code | Probe/mount/query only | No freestanding reader | FAIL | Implement chunk tree, FS tree, directory/extent reads, supported feature checks |
| [exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT) | 0.1.16 | V2-only production; historical workers separate | Namespace read | Freestanding ELF embedded; firmware CI red | FAIL | Resolve external validator availability; exact-release OVMF/corruption qualification |
| [EXT2](https://github.com/Infiltrator-Projects/Filesystem-EXT2) | 1.0.15 | V2-only production; historical V1 source isolated | Namespace read | Freestanding ELF embedded; firmware CI red | FAIL | BOOT package checksum mismatch in current CI; exact-head validator/OVMF |
| [EXT3](https://github.com/Infiltrator-Projects/Filesystem-EXT3) | 1.0.14 | V2-only production; historical V1 source isolated | Clean-volume namespace read | Freestanding ELF embedded; firmware CI red | FAIL | Current validator/OVMF gate; refuse unclean journals safely |
| [FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS) | 1.0.11 | V2-only production; historical V1 source isolated | Namespace read | Freestanding ELF embedded; firmware CI red | FAIL | Current validator artifact / OVMF gate |
| [HFSPLUS](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus) | 1.0.10 | V2-only production; historical source test-only | Native catalog/extent namespace read (qualified subset) | Embedded freestanding ELF; firmware CI red | FAIL | BOOT 0.1.77 has no expected validator; OVMF/release qualification |
| [HFS](https://github.com/Infiltrator-Projects/Filesystem-HFS) | 1.0.11 | V2-only production; historical source test-only | Native catalog/extent namespace read (qualified subset) | Embedded freestanding ELF; firmware CI red | FAIL | Current validator/OVMF gate; MacRoman/overflow support boundaries |
| [MINIX](https://github.com/Infiltrator-Projects/Filesystem-Minix) | 1.0.14 | V2-only production; historical driver source separate | Namespace read | Freestanding ELF embedded; firmware CI red | FAIL | Current validator/OVMF + release gate |
| [NTFS](https://github.com/Infiltrator-Projects/Filesystem-NTFS) | 1.0.18 | V2-only production; historical code test-only | Native MFT namespace read, hosted ntfs-3g/glibc dependency | No no-import freestanding ELF | FAIL | Implement source-owned freestanding MFT/index/runlist reader and tests |
| [PFS3](https://github.com/Infiltrator-Projects/Filesystem-PFS3) | 1.0.10 | V2-only production; historical code test-only | Direct-index PFS1/2 namespace subset | No-import ELF embedded; firmware CI red | FAIL | Validator missing from 0.1.77; OVMF; large-sector/superindex coverage |
| [SFS2](https://github.com/Infiltrator-Projects/Filesystem-SFS2) | 0.1.7 | V2-only production | Native object/B-tree namespace read | Freestanding ELF embedded; no BOOT proof | FAIL | Commit e5cca1bf source+package CI green; real validator+OVMF still absent |
| [SFS](https://github.com/Infiltrator-Projects/Filesystem-SFS) | 1.0.10 | V2-only production | Native object/B-tree namespace read | Freestanding ELF embedded; no BOOT proof | FAIL | Commit 9249561f hosted source and package CI green; validator/OVMF still required |
| [SWAP](https://github.com/Infiltrator-Projects/Filesystem-SWAP) | 1.0.11 | V2-only production | Probe/format; no namespace by design | Correctly no firmware reader | FAIL (expected) | Current CI needs production BOOT validator to prove deliberate refusal; never add fake reads |
| [UFS1](https://github.com/Infiltrator-Projects/Filesystem-UFS1) | 1.0.12 | V2-only production; historical source separate | Native inode/indirect block namespace read | Freestanding ELF embedded; firmware CI red | FAIL | Current production validator/OVMF and release gates |
| [UFS2](https://github.com/Infiltrator-Projects/Filesystem-UFS2) | 1.0.19 | V2-only production; historical source separate | Native inode/indirect block namespace read | Freestanding ELF embedded; firmware CI red | FAIL | Current production validator/OVMF and release gates |
| [XFS](https://github.com/Infiltrator-Projects/Filesystem-XFS) | 1.0.13 | V2-only production; historical code test-only | Probe/mount/query only | No freestanding reader | FAIL | Implement allocation-group inode, extent, directory and CRC-safe firmware reader |
| [ZFS](https://github.com/Infiltrator-Projects/Filesystem-ZFS) | 1.0.10 | V2-only production; historical code test-only | Probe only | No freestanding reader | FAIL | Implement MOS/dataset/dnode/block-pointer/compression/feature-aware reader |

## Direct source repairs made at this checkpoint

- [SFS2 `e5cca1bf`](https://github.com/Infiltrator-Projects/Filesystem-SFS2/commit/e5cca1bf37273f4e2500f887abaf44a33b648460): Native Common V2 mount refuses checksum-valid bitmap marking required live metadata free. Six deliberate corrupt-media cases added to the actual V2 mounted interface. Exact-head hosted and DEB/RUN workflows passed. No firmware release published.
- [SFS `9249561f`](https://github.com/Infiltrator-Projects/Filesystem-SFS/commit/9249561f0a0ebe88acf8ef78d8991ef343911531): Same invariant qualified independently for classic SFS0 metadata/layout with eight checksum-valid corruption cases through the production V2 API. Exact-head hosted source and DEB/native RUN package workflows both passed. No firmware release published.

- [APFS `1ddc6b37`](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/1ddc6b37f1a85831f480971202da874062f90f6e): The actual hosted Common V2 mount now preserves a failed checkpoint read as `IFS_FS_IO_ERROR`, rather than falsely reporting disk corruption. Native code still enforces genuine APFS checkpoint/volume checks. A regression injects an I/O failure after a checksum-valid superblock probe, through CORE.MOUNT. Exact-commit hosted source and DEB/RUN workflows both passed. Full APFS namespace, firmware ELF and BOOT compatibility remain unimplemented.

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

# Independent filesystem Common V2 migration matrix

Source audit checkpoint: **2026-11-13**. Scope: all **23** `Infiltrator-Projects/Filesystem-*` GitHub repositories discovered in the organisation. In addition, the transitional central repository still contains **21** format-labelled directories outside those independent repositories: **19** retain implementation/source files and **2** (VMFS and VMFS6) contain design notes only. They are tracked in the second table; do not count them as independent, working modules. Each filesystem remains independently authoritative for its on-disk format, code, `.so` and `.ko` where applicable. The central `Filesystems` app is an integration host, **not** the canonical home for format decoding.

**Interpretation:** The capability column is a source-level inventory of exported Common V2 implementations located in the independent repository; it is **not** a claim of installed-module readiness, complete filesystem operations, real-hardware qualification, write safety, or release asset publication. Historical integration tests for other interfaces do not qualify Common V2. The adapter column identifies additional code and exports outside the Common V2 descriptor. Standard Common V2 `DEFRAG` operation bits are separate: `MAP` does not imply `PLAN`, `EXECUTE`, `VERIFY` or `RECOVER`.

| Independent filesystem | Observed Common V2 source capabilities | Legacy / ownership boundary | Outstanding work before true V2-only defrag |
|:--|:--|:--|:--|
| [APFS](https://github.com/Infiltrator-Projects/Filesystem-APFS) | CORE probe/mount, VOLUME query | Dedicated worker and fixture remain | Native metadata-backed file MAP, then qualified writer/recovery |
| [Btrfs](https://github.com/Infiltrator-Projects/Filesystem-Btrfs) | CORE probe/mount, VOLUME query (single-device, recorded totals) | Dedicated analyser and transaction code remain | Extent/root ownership, multidevice validation, MAP and safe transactions |
| [EXT2](https://github.com/Infiltrator-Projects/Filesystem-EXT2) | CORE namespace read/write, VOLUME query/sync, DEFRAG MAP | Dedicated worker remains | Independently qualify PLAN/EXECUTE/VERIFY/RECOVER, stage/rollback |
| [EXT3](https://github.com/Infiltrator-Projects/Filesystem-EXT3) | CORE probe/mount, VOLUME query (recorded block totals) | Dedicated journalled writer remains | Native journal-aware MAP, safe transaction/recovery |
| [EXT4](https://github.com/Infiltrator-Projects/Filesystem-EXT4) | CORE probe/mount, VOLUME query (64-bit count validation) | Dedicated extent and journal writer remains | Native extent-tree MAP, journal-aware transactions and recovery |
| [FAT12](https://github.com/Infiltrator-Projects/Filesystem-FAT12) | CORE namespace operations, VOLUME query/check/repair/format | Portable FatFs is V2; older defrag adapter separately present | Native verified MAP/PLAN/EXECUTE/RECOVER through Common V2 |
| [FAT16](https://github.com/Infiltrator-Projects/Filesystem-FAT16) | CORE namespace operations, VOLUME query/sync/format | Portable FatFs is V2; legacy FAT defrag adapter separately present | Native verified MAP and transactional operations through Common V2 |
| [FAT32](https://github.com/Infiltrator-Projects/Filesystem-FAT32) | CORE namespace operations, VOLUME query/sync/format | Common V2 serves filesystem operations; dedicated relocation and recovery remain | Remove retired runtime dependencies only after full MAP/writer parity |
| [FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS) | CORE namespace read/overwrite, VOLUME query/sync, DEFRAG MAP | Dedicated worker and fixture interfaces remain | Close transactional DEFRAG gaps; distinguish old adapter from Common V2 |
| [HFS](https://github.com/Infiltrator-Projects/Filesystem-HFS) | CORE probe/mount, VOLUME query | Legacy HFS worker/fixture source remains | Source-owned file B-tree MAP and proven journal/recovery if applicable |
| [HFSPlus](https://github.com/Infiltrator-Projects/Filesystem-HFSPlus) | CORE probe/mount, VOLUME query (HFS+/HFSX) | Legacy HFS+ worker/fixture source remains | Allocation/catalog/extents MAP, journal/recovery qualification |
| [Minix](https://github.com/Infiltrator-Projects/Filesystem-Minix) | CORE namespace read/write, VOLUME query/sync, DEFRAG MAP | Dedicated transactional worker remains | PLAN/EXECUTE/VERIFY/RECOVER via V2; test all three Minix on-disk formats |
| [NTFS](https://github.com/Infiltrator-Projects/Filesystem-NTFS) | CORE probe/mount, VOLUME query | Older NTFS worker/fixture bridge retained | Runlist-owned file MAP, `$LogFile` and safe transactional recovery |
| [OFS](https://github.com/Infiltrator-Projects/Filesystem-OFS) | CORE namespace read/write/create etc, VOLUME query/sync/check/format | Common V2 serves filesystem operations; dedicated relocation and recovery remain | Native allocation and file-chain MAP, preserve Common V2 BOOT access and verified recovery |
| [PFS3](https://github.com/Infiltrator-Projects/Filesystem-PFS3) | CORE probe/mount, VOLUME query | Older PFS3 worker/fixture code retained | Native file MAP and validated writable/recovery operations |
| [SFS](https://github.com/Infiltrator-Projects/Filesystem-SFS) | CORE probe/mount, VOLUME query | Older SFS worker/fixture code retained | Native object and allocation MAP, planning and recovery |
| [SFS2](https://github.com/Infiltrator-Projects/Filesystem-SFS2) | CORE probe/mount, VOLUME query | Only the Common V2 export in the audited module | Native file/map group plus verified transaction/recovery |
| [Swap](https://github.com/Infiltrator-Projects/Filesystem-Swap) | CORE probe, VOLUME format; no file namespace | Legacy swap CLI analysis exists outside Common V2 SO | Separate V2 swap read-only analysis contract; DEFRAG operations N/A |
| [UFS1](https://github.com/Infiltrator-Projects/Filesystem-UFS1) | CORE probe/mount, VOLUME query, DEFRAG MAP | Legacy writer + staging/recovery maintained separately | Source-owned PLAN/EXECUTE/VERIFY/RECOVER with power-failure qualification |
| [UFS2](https://github.com/Infiltrator-Projects/Filesystem-UFS2) | CORE probe/mount, VOLUME query, DEFRAG MAP | Legacy writer + staging/recovery maintained separately | Source-owned PLAN/EXECUTE/VERIFY/RECOVER with power-failure qualification |
| [XFS](https://github.com/Infiltrator-Projects/Filesystem-XFS) | CORE probe/mount, VOLUME query (v4/v5, CRC-verified v5) | Legacy worker/fixture code retained | Source-owned AG/Btree file MAP; mutation and log consistency |
| [ZFS](https://github.com/Infiltrator-Projects/Filesystem-ZFS) | CORE probe only | Dedicated analyser interface remains | V2-native read-only analysis/MOS + space maps; file DEFRAG generally not applicable |
| [exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT) | CORE namespace operations, VOLUME query/sync/format | Dedicated defragmentation adapter remains | Native extent/bitmap MAP, qualified transactional relocation/recovery |

## Changes made during 13 November 2026 continuation

The independent repositories below were changed on **`main`**; existing format-native read/write, journalling and recovery engines were deliberately left unchanged:

- **EXT3 1.0.9:** [commit `8aa8f72`](https://github.com/Infiltrator-Projects/Filesystem-EXT3/commit/8aa8f72cfd6690f25927bcba197ad82d53eccc5d) adds read-only CORE mount/unmount and VOLUME query using the established canonical EXT3 feature/geometry checks; rejects free blocks greater than total; tests stale handles, busy session and unsupported format.
- **EXT4 1.0.9:** [commit `5afd3f1`](https://github.com/Infiltrator-Projects/Filesystem-EXT4/commit/5afd3f10633f8e774d0b753c3987b9a224347ec2) adds equivalent lifecycle and validates high 64-bit EXT4 block counts when the 64-bit incompat bit is set; tests impossible media geometry.
- **Btrfs 1.0.9:** [commit `ca581cf`](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/commit/ca581cf35ba6031d691d98448b97df9690f126f7) adds CRC32C-based single-device read-only mount/volume query. Reports recorded logical totals only, not independently scanned free extents or multidevice usable space.
- **XFS 1.0.10:** [commit `eace38f`](https://github.com/Infiltrator-Projects/Filesystem-XFS/commit/eace38fb9959328647b45087a007a6c568a39737) adds read-only mount/query, version 4/5 geometry checks and the existing native v5 superblock CRC, rejecting inconsistent recorded free blocks.

**Exact-head qualification now complete for all four:** EXT3 [v1.0.9](https://github.com/Infiltrator-Projects/Filesystem-EXT3/releases/tag/v1.0.9), EXT4 [v1.0.9](https://github.com/Infiltrator-Projects/Filesystem-EXT4/releases/tag/v1.0.9), Btrfs [v1.0.9](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/releases/tag/v1.0.9), and XFS [v1.0.10](https://github.com/Infiltrator-Projects/Filesystem-XFS/releases/tag/v1.0.10). Each independently passed its actual source-tree `.so` tests, package qualification, `.ko` build and immutable publication, with `.deb`, native `.run` and `.ko` assets pointing to the corresponding source commit above. A [hosted four-format real-image run #43312877572](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/43312877572) additionally passed authoritative `CORE/VOLUME` real-media read-only checks and actual EXT/XFS worker routing, no-write refusals and source-image digests. These are read-only capabilities, **not** DEFRAG relocation/recovery qualification.

## Native Defragmenter worker routing (13 November 2026)

The current production worker source at [Defragmenter commit `1fc58e9`](https://github.com/Infiltrator-Projects/Defragmenter/commit/1fc58e94c09b58a6e1b9160e253a6a1070edc55f) routes explicit read-only `volume-info` to independent Common V2 providers for FAT12/16/32, EXT2/3/4, XFS, Btrfs, APFS, NTFS, classic HFS, HFS+, Minix, PFS3, SFS/SFS2, OFS and exFAT, with source-owned `CORE.PROBE` selecting formats where necessary. Other worker commands retain their original analyser/writer/recovery code; **none** has been reclassified as complete V2 DEFRAG unless a native provider advertises and verifies the precise capability.

[Hosted worker compilation #43314637966](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/43314637966) passed all **13** production worker targets under `-DLD_ENABLE_WERROR=ON` and preserved the old transactional writers. This is a **compile** qualification; full runtime / recovery testing for every one of those worker adapters is still outstanding. The separate [real-media four-format check #43312877572](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/43312877572) provides substantive EXT/XFS runtime evidence. At the last checkpoint the Defragmenter full self-hosted project quality gate and sanitizers for this worker revision were still running; source audit has been reviewed separately in `Defragmenter/docs/AUDIT_STATUS.md`.

## Prior already-qualified progress

- UFS1 [v1.0.11](https://github.com/Infiltrator-Projects/Filesystem-UFS1/releases/tag/v1.0.11): genuine read-only Common V2 `DEFRAG.MAP` using native UFS1 inode inventory; synthetic on-disk inode/dirent tests and independent CI qualified.
- UFS2 [v1.0.16](https://github.com/Infiltrator-Projects/Filesystem-UFS2/releases/tag/v1.0.16): genuine read-only V2 `DEFRAG.MAP`, additionally tested through Defragmenter against a real UFS2 image with >32 extents, negative corruption tests and enforced non-mutation.
- Defragmenter [v1.8.0-299](https://github.com/Infiltrator-Projects/Defragmenter/releases/tag/v1.8.0-299): green project quality gate and published at that checkpoint. Its current success does **not** establish an entirely V2-only codebase.

## Additional central-only filesystem source and designs — extraction backlog

These are **not** included in the 23 independent modules above. The table was obtained from the actual tracked `native/filesystems/<id>/` tree of the transitional [Filesystems repository](https://github.com/Infiltrator-Projects/Filesystems) as of **13 November 2026**, not from speculative format names. The source counts include documentation/build metadata alongside implementation files; they do **not** mean a usable `.so`, `.ko`, `.efi`, boot loader, mount implementation or successful Common V2 test exists. The first 15 named "Linux import manifest" appear in `data/imported-source-filesystems.txt`; the others exist in the tree but are not listed there.

| Central format | Tracked files | Current evidence | Required before claiming independently qualified Common V2 |
|:--|--:|:--|:--|
| [adfs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/adfs) | 13 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [bcachefs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/bcachefs) | 246 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [befs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/befs) | 19 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [bfs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/bfs) | 9 | Linux source; not in import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [cpm](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/cpm) | 56 | Userspace source; not in import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [efs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/efs) | 10 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [f2fs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/f2fs) | 32 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [fosfat](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/fosfat) | 36 | Userspace source; not in import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [gfs2](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/gfs2) | 48 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [hpfs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/hpfs) | 18 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [jfs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/jfs) | 48 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [nilfs2](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/nilfs2) | 43 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [ocfs2](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/ocfs2) | 115 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [omfs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/omfs) | 9 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [qnx4](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/qnx4) | 8 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [qnx6](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/qnx6) | 8 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [reiserfs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/reiserfs) | 33 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [sysv](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/sysv) | 12 | Linux import manifest | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [udf](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/udf) | 24 | Linux source; dedicated UDF test workflow, no independent Git | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [vmfs](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/vmfs) | 1 | Design-only; no implementation | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |
| [vmfs6](https://github.com/Infiltrator-Projects/Filesystems/tree/main/native/filesystems/vmfs6) | 1 | Design-only; no implementation | Establish canonical independent source ownership, portable host `.so` ABI and native tests, then qualify real-media read-only operations, kernel module and any safe writer/recovery |

**Source-only:** 19 formats (the first 19 rows) have a nontrivial central implementation/source tree but no independently verified Common V2 `.so` in the 23-repository inventory. **Design-only:** VMFS and VMFS6 each currently consist solely of `DESIGN.md`. Never label either as a supported operational filesystem.

**Reconciliation rule:** Before extracting a central-only format, compare its full source files with any existing independent implementation or actual upstream import, retain attribution/licences, assign exactly one format owner, and qualify the portable module and source-owned kernel component separately. Do not move code into Defragmenter or treat imported Linux kernel sources as automatically available in a freestanding `.so`.

## Release gates and non-negotiable acceptance criteria

1. **Producer-owned code:** Format-specific parsing, allocation, extent translation, journalling, mutation and recovery stay in the independent filesystem repository. The Defragmenter host only performs authenticated provider discovery, target identity and mount checks, cancellation, signed action consent and policy enforcement.
2. **V2 capabilities must be true:** advertise only actual implemented operations, with real negative tests for unsupported paths and malformed/mounted/changed media; never stub a V2 call or fall back to a different interface.
3. **Functional parity before retiring dedicated adapters:** for a format with legacy native analyse, compact, growth/defrag, format or recovery functionality, port and test it through real Common V2 entry points **before** deleting the legacy implementation; preserve failure/recovery guarantees and on-disk data.
4. **Evidence per format:** build and test actual current `.so` with `-Werror`; create an independently verified disposable filesystem image; exercise its V2 consumer in Defragmenter; test corrupt geometry and cross-format rejection, no-write/read-only mode, cancellation and modified target identity; for writers include power-loss replay, verify and rollback. Check independent `.ko` compilation/ABI, release `.deb`, native `.run` and documentation separately. An absent capability must remain unavailable.
5. **Release integrity:** all changes go to the existing `main`; no PR branches, reporting old integration tests as Common V2 evidence, artificial CI passes, installed filesystem writes or new Python dependencies.

Central issue for follow-through: [Filesystems #80](https://github.com/Infiltrator-Projects/Filesystems/issues/80). This matrix records the whole set but **does not** mean the 23 separate migrations have completed.


## 22 November 2026: 23-repository Common V2 qualification ledger

This is a **source and GitHub Actions snapshot**, not a result copied from
Zabbix. The current BOOT implementation is not yet Common V2-capable: **none of
these modules is declared BOOT-compatible here**. A module can implement both
hosted and freestanding V2 now and still be unqualified for BOOT execution.
The former installed-validator pass/fail count of 5/18 has not been replayed
against the original binaries, and is not substituted by source inspection or
green build jobs. SWAP is intentionally nonbootable.

The ABI column records source-level Common V2 exports; the hosted/firmware
columns record implemented source profiles, **not** acceptance by a production
BOOT loader. "RO subset" means native on-disk operations and
unsupported-feature rejection, not unlimited coverage of every format variant.
"Carrier" means the sole installed .so itself is a freestanding module;
"embedded" means the installed .so contains the freestanding ELF in its
`.infiltrator_boot` section. The versions below are repository VERSION
files inspected on 22 November; existing release tags may be older.
Main workflow observations precede the APFS changes listed below.

| Independent repository | ABI source | Hosted V2 operations | Firmware V2 source | Actual current BOOT V2 validator | GitHub tests / build at checkpoint | VERSION | Remaining format-specific qualification |
|:--|:--|:--|:--|:--|:--|:--|:--|
| Filesystem-APFS | V2 | RO subset STAT/LIST/READ; constrained catalog | Embedded ELF, RO subset | Not executable against current BOOT | Previous main green; 22 Nov corruption-fix CI queued | 1.0.13 | Checkpoint/tree-height, encrypted, snapshots/clone feature subsets; packaged media and independent UEFI handoff |
| Filesystem-Btrfs | V2 | RO subset STAT/LIST/READ | Embedded ELF, RO subset | Not executed | Main hosted/firmware media CI green | 1.0.12 | Multidevice/chunk profiles, unsupported tree features and real UEFI handoff |
| Filesystem-EXT2 | V2 | RO namespace and native engine | Embedded ELF | Not executed | Main SO and package CI green | 1.0.16 | Exact packaged binary/OVMF handoff and unsupported feature cases |
| Filesystem-EXT3 | V2 | RO namespace on clean supported journals | Embedded ELF | Not executed | Main native/package CI green | 1.0.15 | Journal dirty/replay denial, OVMF and shipped module proof |
| Filesystem-EXT4 | V2 | RO namespace, extents and indirect blocks | Embedded ELF | Not executed | Main SO and release CI green | 1.0.19 | Unsupported features and exact packaged firmware execution |
| Filesystem-FAT12 | V2 | Native FAT namespace | Freestanding single-file SO | Not executed | Main release CI green | 0.1.30 | Firmware path and exact kernel/initramfs byte comparison |
| Filesystem-FAT16 | V2 | Native FAT namespace | Freestanding single-file SO | Not executed | Main release CI green | 0.1.22 | Firmware path and exact kernel/initramfs byte comparison |
| Filesystem-FAT32 | V2 | Native FAT namespace | Freestanding single-file SO | Not executed | Main release CI green | 0.1.18 | Firmware path and exact kernel/initramfs byte comparison |
| Filesystem-FFS | V2 | Native Amiga read-only namespace; limited write separately | Embedded ELF | Not executed | Main freestanding/package CI green | 1.0.13 | Feature-specific real media and firmware handoff |
| Filesystem-HFS | V2 | Classic HFS RO namespace | Embedded ELF | Not executed | Main hosted/firmware/package CI green | 1.0.13 | MacRoman, overflow extents and current BOOT-independent firmware execution proof |
| Filesystem-HFSPlus | V2 | HFS+/HFSX RO subset | Embedded ELF | Not executed | Main hosted/firmware/package CI green | 1.0.12 | Overflow extents, Unicode collation and real UEFI handoff |
| Filesystem-Minix | V2 | Minix V1/V2/V3 native namespace | Embedded ELF | Not executed | Main freestanding/SO/package CI green | 1.0.16 | All supported on-disk variants and exact packaged UEFI qualification |
| Filesystem-NTFS | V2 | Native NTFS RO namespace | Embedded ELF | Not executed | Main SO/package CI green | 1.0.20 | Unclean/logged media, unsupported attributes and UEFI byte comparison |
| Filesystem-OFS | V2 | Native Amiga namespace | Freestanding single-file SO | Not executed | Main publication CI green | 0.1.17 | Block size/feature boundaries, boot read execution |
| Filesystem-PFS3 | V2 | PFS1/2 direct-index RO subset | Embedded ELF | Not executed | Main hosted/firmware/package CI green | 1.0.12 | Superindexes/large files and OVMF production handoff |
| Filesystem-SFS | V2 | SFS0 RO namespace | Embedded ELF | Not executed | Main hosted/package CI green | 1.0.11 | Exact package and production validator plus UEFI handoff |
| Filesystem-SFS2 | V2 | SFS2 RO namespace | Embedded ELF | Not executed | Main hosted/package CI green | 0.1.8 | Exact package and production validator plus UEFI handoff |
| Filesystem-Swap | V2 | SWAPSPACE2 PROBE / FORMAT; no file namespace | Not applicable: no files | **N/A — must reject as boot volume** | Main media/package CI green | 1.0.12 | Maintain no namespace, no firmware profile and no fake boot capability |
| Filesystem-UFS1 | V2 | Native UFS1 RO namespace / MAP | Embedded ELF | Not executed on current BOOT | Main SO/firmware/package CI green | 1.0.13 | Independent current packaged UEFI handoff and variant boundaries |
| Filesystem-UFS2 | V2 | Native UFS2 RO namespace / MAP | Embedded ELF | Not executed on current BOOT | Main SO/firmware/package CI green | 1.0.20 | Independent current packaged UEFI handoff and variant boundaries |
| Filesystem-XFS | V2 | V4/V5 RO namespace and journal cleanliness checks | Embedded ELF | Not executed | Main native/firmware/package CI green | 1.0.14 | Supported AG, Btree, dirty-log and exact packaged UEFI qualification |
| Filesystem-ZFS | V2 | Pool/label PROBE only | Freestanding PROBE only; **no namespace** | Not applicable for current payload | Main pool probe/independent SO CI green | 1.0.10 | Native dataset and ZAP/dnode file namespace with real file reads, then boot-file tests |
| Filesystem-exFAT | V2 | Native exFAT namespace | Freestanding V2 source profile | Not executed | Main native/freestanding CI green | 0.1.17 | Exact packaged image, corruption and UEFI handoff |

### Changes committed in this checkpoint

- APFS [`530b711`](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/530b71109e71b16548a8f1dbd8a3693147471012): a checksummed catalogue with two identical parent/name directory keys now returns `IFS_FS_CORRUPT` rather than choosing the first child.
- APFS [`2b85b7a`](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/2b85b7ac583684b99dc4b81c45d877710f5ce4c7): exercise the duplicate directory corruption via the same firmware/Common V2 ABI.
- APFS [`7756b3d`](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/7756b3d522b64b8bc630559aed46d5a9cc422b31) and [`654865c`](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/654865c601b574dfa2fdfe8493e77e8afb37156a): duplicate inode keys fail closed and gain an independent checksummed V2 corruption case.
- These changes touch only the independent filesystem code and CI tests; **BOOT is unchanged**.
- Do not mark the repairs qualified until the GitHub workflow on the last APFS main SHA completes successfully. A subsequent production BOOT validator/OVMF test belongs to the separate future BOOT integration qualification.

### 22 November follow-up — additional source repairs

- Btrfs [`5535fc5`](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/commit/5535fc5c94257c96132d838d0b0c20f39b42b4ae): full DIR_INDEX / DIR_ITEM exact-name scan now returns `IFS_FS_CORRUPT` on two entries with the same pathname rather than first-match success; hosted and firmware compile the same reader.
- Btrfs [`fbc74f3`](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/commit/fbc74f3bb4a334c573ffe2d80eb919c27a1e5b2a) and [`f2b3c5d`](https://github.com/Infiltrator-Projects/Filesystem-Btrfs/commit/f2b3c5df2b1051aadbb3aee9dac182e248cac2b8): native duplicate-record tests are a separate mandatory CTest alongside the existing real-media Common V2 tests. Their exact-head Actions were still queued at this checkpoint.
- NTFS [`67b3b52`](https://github.com/Infiltrator-Projects/Filesystem-NTFS/commit/67b3b5222cd8265e4bbc8841e7815fa36e90982e): a matching NTFS FILE attribute is now returned only after the full attribute chain and terminator have been checked; duplicate unnamed attributes and subsequent corruption fail closed.
- NTFS [`f6cfb4a`](https://github.com/Infiltrator-Projects/Filesystem-NTFS/commit/f6cfb4a89d618727c183c9543b2080fe36a176b1) and [`ae8687d`](https://github.com/Infiltrator-Projects/Filesystem-NTFS/commit/ae8687dfaeee77586696922906599f1b56332a4c): add a corruption regression to the existing full, real-media V2 suite. Exact-head Actions were still queued.
- APFS [`355bda0`](https://github.com/Infiltrator-Projects/Filesystem-APFS/commit/355bda0778e0bf434b10b8f14b36bc1761066919) and XFS [`086c1d0`](https://github.com/Infiltrator-Projects/Filesystem-XFS/commit/086c1d08df3d381b221c8530edc79540333bfb17) reconcile stale source-status statements with the actual existing hosted/freestanding V2 readers; both retain unqualified BOOT integration status.
- None of these source repairs changes BOOT, Secure Boot, an ESP, NVRAM, Zabbix or an installed media image. None is marked passed or released until its exact-head gates finish.

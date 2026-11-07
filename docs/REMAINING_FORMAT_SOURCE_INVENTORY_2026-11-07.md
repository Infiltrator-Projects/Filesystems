# Remaining format source inventory and extraction safeguards — 2026-11-07

**Source snapshot:** [Filesystems `529c9370dd0a`](https://github.com/Infiltrator-Projects/Filesystems/commit/529c9370dd0a31c05009461b992ecf9efab130f6). **Disposition: inventory only; no code migrated or deleted by this audit.**

This is an evidence-backed loss-prevention inventory for the **21 manifest identities** under `native/filesystems/`, not an assertion that all 21 have independent working implementations. The user-approved design is **one on-disk format per independent repository and one format-specific distributable `.so`**. The central `Filesystems` repository is **not yet retired**, and does **not** become a submodule aggregator. Preserve tested code, provenance, imported licences and upstream notices until each independent repo is a complete and verified source owner.

The repository search on 2026-11-07 found **no matching `Filesystem-*` standalone repositories for the 21 rows below**. The proposed owner strings are **names to create eventually**, not working GitHub projects or confirmed migration destinations. Do not install anything from these names until they actually exist and qualify.

## Current per-format source

| Format ID | Tracked blobs | C/C++/asm source & headers | Source class | Future standalone owner |
| --- | ---: | ---: | --- | --- |
| `adfs` | 13 | 10 | Imported Linux | `Filesystem-ADFS` (not created) |
| `bcachefs` | 246 | 243 | Imported Linux | `Filesystem-BCACHEFS` (not created) |
| `befs` | 19 | 15 | Imported Linux | `Filesystem-BEFS` (not created) |
| `bfs` | 9 | 6 | Native module / Linux | `Filesystem-BFS` (not created) |
| `cpm` | 56 | 19 | Userspace code | `Filesystem-CPM` (not created) |
| `efs` | 10 | 7 | Imported Linux | `Filesystem-EFS` (not created) |
| `f2fs` | 32 | 29 | Imported Linux | `Filesystem-F2FS` (not created) |
| `fosfat` | 36 | 18 | Userspace code | `Filesystem-FOSFAT` (not created) |
| `gfs2` | 48 | 45 | Imported Linux | `Filesystem-GFS2` (not created) |
| `hpfs` | 18 | 15 | Imported Linux | `Filesystem-HPFS` (not created) |
| `jfs` | 48 | 45 | Imported Linux | `Filesystem-JFS` (not created) |
| `nilfs2` | 43 | 40 | Imported Linux | `Filesystem-NILFS2` (not created) |
| `ocfs2` | 115 | 109 | Imported Linux | `Filesystem-OCFS2` (not created) |
| `omfs` | 9 | 6 | Imported Linux | `Filesystem-OMFS` (not created) |
| `qnx4` | 8 | 5 | Imported Linux | `Filesystem-QNX4` (not created) |
| `qnx6` | 8 | 5 | Imported Linux | `Filesystem-QNX6` (not created) |
| `reiserfs` | 33 | 29 | Imported Linux | `Filesystem-REISERFS` (not created) |
| `sysv` | 12 | 9 | Imported Linux | `Filesystem-SYSV` (not created) |
| `udf` | 24 | 21 | Native module / Linux | `Filesystem-UDF` (not created) |
| `vmfs` | 1 | 0 | Design only | `Filesystem-VMFS` (not created) |
| `vmfs6` | 1 | 0 | Design only | `Filesystem-VMFS6` (not created) |

Totals: **789 format-directory blobs**; **676 C/C++/assembly source/header files**; **19 formats contain real source**, while **2 formats have no implementation files here**.

- `vmfs` and `vmfs6` contain **only `DESIGN.md`**. They must **not** be advertised as extracted, working, or buildable drivers. First locate/import actual source and independently qualify any original implementation.
- `cpm` and `fosfat` preserve userspace code and accompanying licence/readme files, not kernel trees.
- `bfs` and `udf` are the only remaining formats listed by `data/native-filesystems.tsv`; their current native-management state must be separately tested. Source presence alone proves neither working `.ko` nor runtime safety.
- The other 15 format directories contain imported Linux implementation trees, as recorded by `data/imported-source-filesystems.txt`. Do **not** delete or overwrite the copied code until authorship, licence and full functionality are migrated.

## Separate UEFI firmware loss-prevention inventory

**71 tracked blobs** also remain under `firmware/uefi/filesystems/`. They include imported format-specific EFI readers and shared infrastructure, even for formats whose Linux/`.so` source has already moved to standalone repos. Examples:

- `firmware/uefi/filesystems/fsw_btrfs.c`
- `firmware/uefi/filesystems/fsw_btrfs_zstd.h`
- `firmware/uefi/filesystems/fsw_ext2.c`
- `firmware/uefi/filesystems/fsw_ext2.h`
- `firmware/uefi/filesystems/fsw_ext2_disk.h`
- `firmware/uefi/filesystems/fsw_ext4.c`
- `firmware/uefi/filesystems/fsw_ext4.h`
- `firmware/uefi/filesystems/fsw_ext4_disk.h`
- `firmware/uefi/filesystems/fsw_hfs.c`
- `firmware/uefi/filesystems/fsw_hfs.h`
- `firmware/uefi/filesystems/fsw_ntfs.c`
- `firmware/uefi/filesystems/fsw_reiserfs.c`
- `firmware/uefi/filesystems/fsw_reiserfs.h`
- `firmware/uefi/filesystems/fsw_reiserfs_disk.h`

**Do not equate an extracted format's Linux shared object with complete firmware migration.** Before deleting any of these EFI sources, verify the exact corresponding code and its required headers/build/test fixtures exist, with licence provenance, in a separately qualified format owner or generic BOOT repository. Do **not** repoint `Filesystems` to standalone Git submodules as a shortcut. EFI runtime compatibility and the long-term single-distributable-SO loader are separate qualification questions.

## Extraction procedure (one format at a time)

1. Identify the **best existing implementation**, imported code licence and all coupled source/headers/tests from both the format directory and any overlapping EFI/legacy paths. Archive the exact source SHA and identify external provenance.
2. Create or verify the format's true independent repository. Bring over all required format-specific code and historical design requirements, preserve credits and third-party licences, then implement **only one** format-specific distributable `.so`, any separately qualified `.ko`/EFI transition paths, complete user manual and native installer.
3. Run build, ABI/format identity, independent read/write/format/defrag capabilities **as genuinely supported**, real-media fixture and safe rollback checks. Do not claim untested operations work. Confirm its installer installs no other format's `.so`.
4. Only once equivalence and independent release are documented, **delete** the old source/build/helper references from `Filesystems` (do not replace with submodules or wrapper copies). Update `data/source-filesystems.txt`, `data/imported-source-filesystems.txt` where applicable, `data/native-filesystems.tsv`, CI, documentation, and the persistent handover.
5. Re-run the central manager's source-boundary check, C++ build and action-policy/desktop-action tests, plus the independent repo release tests.

**Do not delete `Filesystems` itself yet.** This table and [continuity issue #79](https://github.com/Infiltrator-Projects/Filesystems/issues/79) are durable migration controls, not release claims.

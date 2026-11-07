# Filesystems extraction: durable handover and recovery point (2026-11-07)

**Status: IN PROGRESS. DO NOT ARCHIVE OR DELETE THIS REPOSITORY.**

This document is the continuity record for extracting filesystem implementations from the temporary `Filesystems` manager into independently versioned `Filesystem-*` Git repositories. Keep it with source control: future work must use this file and the tracked issue rather than assuming any chat transcript survives.

## Fixed architecture decision

- **Exactly one authoritative filesystem implementation and one distributable `.so` for each on-disk format.** `FFS.so`, `exFAT.so`, `FAT12.so`, `FAT16.so`, `FAT32.so`, etc. must remain separate. The word *unified* means operations (read/write/format/defrag/recover/Test Media where supported) of **one filesystem** in **its own** `.so`. It never means combining formats into a universal shared object.
- The standalone filesystem repository owns its format-specific implementation, tests, installer, manual/PDF, release, and separately qualified kernel/EFI adapters where applicable. Common/Libraries owns truly format-neutral interfaces only; application hosts provide I/O and safety services, not duplicate filesystem-specific engines.
- The central `Filesystems` source implementation is extracted then **removed**; do **not** redirect it into central provider Git submodules, symlink forests or vendored mirrors. No new `native/providers/Filesystem-*` entries.
- `Filesystems` itself **must remain for now** as a transitional manager and source owner of unextracted formats. Only retire it after all remaining implementations and consumers have been migrated, qualified, and separately tested.
- Preserve working releases, license notices, source history, and real-media safety limits. Never replace a working implementation or modify physical media as a speculative migration test.

## Verified recovery points (2026-11-07, Australia/Melbourne)

| Repository | `main` revision before this handover | Version | Verified status |
| --- | --- | --- | --- |
| [Filesystems](https://github.com/Infiltrator-Projects/Filesystems) | `9d419e7fedfd442b519abfabd8ee435a8c9d8d77` | `0.5.116` | Retired source/helper boundaries, provider identities, read-only independent package checker, three CTests and published FFS/exFAT **sandbox dpkg installation/removal** all PASS ([verification](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42359837292)). **Actual GTK/host runtime not yet accepted; 0.5.115 not published.** Last central release `v0.5.110`. |
| [Filesystem-FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS) | `dc62ac594c6959b6fd03093a30fb9c6e0fdc9b1c` | `1.0.7` | [`v1.0.7`](https://github.com/Infiltrator-Projects/Filesystem-FFS/releases/tag/v1.0.7) **published**, independent FFS SO and source-RUN/package workflow PASS, includes `ffs-linux-6.12.107.ko`, native `.run`, `.deb`. |
| [Filesystem-exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT) | `02d054202e4bbb2b3f1da36391019b64975842cb` | `0.1.10` | [`v0.1.10`](https://github.com/Infiltrator-Projects/Filesystem-exFAT/releases/tag/v0.1.10) **published**, standalone SO, defrag, kernel/QEMU and package workflow PASS; assets include `exFAT.so`, `exfat-linux-6.12.107.ko`, `.run`, `.deb`. |

GitHub commits are durable recovery points; do not confuse a successful *temporary manager compilation* with a functional installed application.

Prior extracted-source commits on Filesystems main:
- [`99f29cb`](https://github.com/Infiltrator-Projects/Filesystems/commit/99f29cb4f8e06db4eb427b42b925ff5c94f9463f): remove 23 filesystem submodule links, central FAT implementation and extracted source placeholders; remove direct builds.
- [`f0e8d6e`](https://github.com/Infiltrator-Projects/Filesystems/commit/f0e8d6ec7aa8c42f67f02b49b297310a46fc25e2): retire obsolete per-format build/test helpers and **stop central Filesystems APT publication**.
- [`df93e6a`](https://github.com/Infiltrator-Projects/Filesystems/commit/df93e6a923d3559b1cf7e593c6bd48f1fed2d7b6): remove `fsinspect` format engine linked to extracted Amiga cores; [boundary + manager build PASS](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42327315107).
- [`d6bf9d4`](https://github.com/Infiltrator-Projects/Filesystems/commit/d6bf9d4ef85dfcd29b1defc6528dba3ac7803991): correct source inventories and flag historical documentation; [boundary + manager build PASS](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42349589099).

## Known unfinished work — next actions IN ORDER

- [x] **Remove retired central helper routes and misleading GUI install/remove buttons:** FAT12/FAT16/FAT32/OFS/EXT2/Minix C++ helper references, their actions and obsolete GUI controls were deleted; the catalogue and BFS/UDF native operations remain. OFS/FAT16 show disabled `Independent package` labels. Source contract, full temporary GTK manager compile and both action-policy/desktop-action CTests passed ([verified run](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42356485052), [code change](https://github.com/Infiltrator-Projects/Filesystems/commit/5dee50ac8e8b0e0b793db067372d02d7c302026d)). **No on-disk filesystem was touched.**
- [x] **Independent package status/identity integration implemented and compiled:** `src/independent_package.{hpp,cpp}` verifies installed Debian package metadata, exclusive file ownership, regular-file policy, SHA-256, ELF type, and per-format exported ABI or embedded marker where the standalone format supports it. `src/probe.cpp` reports **Package verified** separately from **Ready**; if proof fails, it reports Incomplete and does not expose central helper actions. Eleven extracted format catalogue entries are `SupportProvider::Independent`, including FAT12/FAT16/FAT32 as three separately verified modules under the classic-FAT row. [Implementation and full CI](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42359837292); [source commit](https://github.com/Infiltrator-Projects/Filesystems/commit/1cd54e47b472334ad09b9eaa815e26215ca14cd9). **Proof does not mean runtime mount, formatting, boot or recovery qualified.**
- [x] **FFS/exFAT Debian package coexistence proven in a disposable private dpkg root:** exact release SHA-256 and source revision, separate .so/PDF payloads, format-specific ABI exports, package installation, mutual non-overlap, removal and reinstall without deleting the other package. [Isolated test CI PASS](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42359837292), [code](https://github.com/Infiltrator-Projects/Filesystems/commit/2898babc6ceb764b97abd0e819b67342c059cc3a). No host disk or real /System directory modified. This is a sandbox package transaction test, **not** an installed GTK interface acceptance test.
- [x] **Real Linux Mint desktop smoke test (Filesystems 0.5.115):** captured GUI screenshots confirm the overview works and the correct executable starts. Amiga OFS/FFS/SFS/SFS2/PFS3 and Microsoft FAT12/FAT16/FAT32/exFAT/NTFS show **Package verified** and disabled **Independent package** buttons; old central Install/Remove actions no longer appear. [Evidence in issue #79](https://github.com/Infiltrator-Projects/Filesystems/issues/79#issuecomment-6705314907). On the test laptop FFS 1.0.6 and exFAT 0.1.9 are *installed*, older than their published FFS 1.0.7 and exFAT 0.1.10 releases. No update or physical media operation was performed.
- [x] **FAT12/FAT16/FAT32 independently packaged, sandbox qualified:** [CI run 42361937020](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42361937020) passed all four jobs: source extraction boundary, manager build, prior FFS/exFAT package isolation and new FAT-family package isolation. Published FAT12 0.1.13, FAT16 0.1.14 and FAT32 0.1.10 Debian packages are pinned by SHA-256, checked against separate `.so`/manual/format marker/ABI, installed/removed independently under an isolated temporary root, with the two other FAT installations surviving each removal. [Test source](https://github.com/Infiltrator-Projects/Filesystems/commit/a4aa7b284cf72704def4ae3e1a00b1463ed5e9d4).
- [x] **Negative release artifact identity tests (isolated):** the actual published FFS, exFAT, FAT12, FAT16 and FAT32 module copies are tested for missing file, appended-byte tampering, non-ELF content, and replacement with a foreign format's SO **even when the expected hash is set to match that foreign file**. Format-specific ABI symbols or .ifsmeta identities reject the foreign copy. Restored copies then pass and independent Debian install/remove tests still pass. [All four CI jobs PASSED](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42363094071), [test commit 66af86c](https://github.com/Infiltrator-Projects/Filesystems/commit/66af86c5357ad28cc81820900df4a14127dbc515). This tests released-package validators, **not an injected production dpkg-query runtime fixture**; no physical media touched.
- [x] **C++ manager package verifier tested against an isolated simulated Debian package database and real synthetic ELF fixture files:** `independent_package.cpp` now has an explicit read-only backend seam. Production remains pinned to trusted `dpkg-query`, `readelf` and `/System/Filesystems` paths; CTest injects a fake dpkg response backend and disposable FFS/exFAT/FAT12/FAT16/FAT32 ELF files. It verifies genuine package status, each independent FAT module, and rejection of missing/half-configured metadata, missing listing/owner, readelf failure, changed content, non-ELF, symlinks, and **foreign ELF despite matching forged SHA-256**. [All four CI jobs PASS](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42364066910), [C++ test source](https://github.com/Infiltrator-Projects/Filesystems/blob/main/tests/independent_package_runtime_test.cpp). Source version **0.5.116**. This does not involve host dpkg, disks or production deployment.
### Host upgrade preflight — 2026-11-07, Dell Latitude 5550

Shannon ran a read-only installation/launcher/module inventory on Linux Mint:

- `dpkg-query` reported `filesystems 0.5.110 install ok installed`.
- `/usr/bin/filesystems` is the installed, dpkg-owned compatibility launcher;
  `/Programs/Filesystems/Filesystems` is the installed, dpkg-owned executable
  (approximately 438 KiB), both dated 2026-11-05.
- `~/Filesystems-GUI-Test/build/Filesystems` is a separately compiled,
  unowned local executable (approximately 430 KiB, dated 2026-11-07).
  It is not the executable selected by the desktop launcher.
- `/usr/share/applications/org.infiltrator.Filesystems.desktop` contains
  `Exec=filesystems`, so launching through the existing menu still uses
  the dpkg-installed application.
- All **23** independent filesystem modules were present directly under
  `/System/Filesystems/` and were successfully SHA-256 enumerated:
  APFS, BTRFS, exFAT, EXT2, EXT3, EXT4, FAT12, FAT16, FAT32, FFS,
  HFSPLUS, HFS, MINIX, NTFS, OFS, PFS3, SFS2, SFS, SWAP, UFS1, UFS2,
  XFS and ZFS. The full original checksum listing was recorded locally
  by the user's `upgrade-preflight.txt` session; this summary is not a
  cryptographic ownership/qualification claim.
- The `/Temporary/InfiltratorOS/Kernel` workspace and its build outputs
  belong to a separate Kernel workflow and are not affected by this
  Filesystems manager preflight.

**Conclusion:** the old GUI is still installed, the newer GUI remains a
separate test executable, and no module removal or replacement is evidenced.
Do not confuse current source version 0.5.116 or a passing GTK compile with
a published installable manager release. The latest published central
Filesystems manager release remains 0.5.110; central package publication was
retired in the extraction cleanup. The next qualification gate is an
isolated old-to-new manager Debian upgrade, including launcher/menu ownership
and strict proof that the 23 independently owned modules are untouched.
Do not test this by removing the production package (`prerm remove`
invokes module cleanup); a real deployment requires separately verified
package bytes and explicit acceptance.

- [x] **Actual Debian manager upgrade qualified off-machine:** [all checks PASS](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42368127393) on a disposable GitHub-hosted Ubuntu 24.04 machine. Built Filesystems 0.5.116 with `/usr` launcher prefix, installed the real public 0.5.110 manager, then upgraded in place using `apt-get --no-remove`. Verified actual `0.5.116` executable and menu ownership, GTK start via Xvfb, **all 23 format `.so` SHA-256 values unchanged**, and five real FFS/exFAT/FAT independent Debian package ownership records intact. The other 18 were per-format integrity canaries. [Test implementation](https://github.com/Infiltrator-Projects/Filesystems/blob/main/tests/manager_upgrade_acceptance.py) and [CI artifact](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42368127393) (14-day retained, not an APT publication).
- [ ] **Next: local Mint upgrade, only after user-side build and simulated APT plan:** source `0.5.116` now has a qualified Debian package procedure, but Shannon's installed desktop menu still targets `0.5.110`; the GitHub candidate is not an APT release. On user laptop, regenerate `.deb` from the current main source using `-DCMAKE_INSTALL_PREFIX=/usr`, inspect staged package for `/System/Filesystems` absence, run `apt-get -s install` with `--no-remove`, and preserve `upgrade-preflight.txt` and the 23-module SHA-256 baseline before an in-place upgrade. **Never remove or purge the old package**—legacy `prerm remove` can clean active native modules. After explicit local upgrade, verify package version, desktop launcher and `sha256sum -c` against the saved baseline. Real-media read/write, kernel/boot/recovery and remaining-format extraction remain separate.
- [x] **Inventory completed, no source deleted:** [remaining 21-format source and 71-blob UEFI audit](docs/REMAINING_FORMAT_SOURCE_INVENTORY_2026-11-07.md) identifies 789 format-directory files, 676 source/headers and 19 formats with implementation code. `vmfs` and `vmfs6` are only design documents; no new standalone repo exists yet for these 21 identities. [Inventory commit](https://github.com/Infiltrator-Projects/Filesystems/commit/3b1efc2ca5123538c70a5358e3d73c1ddc3fb48d).
- [ ] **Extract remaining formats one at a time.** For each, verify an independent real repository containing complete best source, provenance, tests, correct format-specific `.so`, qualified optional `.ko`/EFI path, manual and passing release. **Only then delete** the old format code from central and update manifests/build/tests. Also resolve the 71 firmware blobs before removing overlapping format-specific firmware.
- [ ] Keep `data/native-filesystems.tsv` consistent with remaining centrally-owned module code (currently BFS and UDF), but do not assume a .ko is qualified simply because source exists.
- [ ] Audit historical `docs/*` and old open issues for extracted formats. Move authoritative knowledge/bugs to the corresponding standalone repo; keep traceable links rather than losing historical design requirements. Historical issue numbers #61, #45 and OFS #35/#33/#31/#29/#27 still refer to past central work.
- [ ] Coordinate InfiltratorOS, Defragmenter, Files, BOOT, Setup and kernel consumers using the agreed per-filesystem ABI. A per-format `.ko` or `.efi` is currently a separate qualified compatibility path; a Linux userspace `.so` cannot be executed directly by an ordinary kernel/UEFI loader.
- [ ] **Only at the end:** establish and pass end-to-end installer, upgrade, removal, generic loader and recovery tests; then decide when central `Filesystems` can be retired. Do not delete its repository or make its old release disappear prematurely.

## Local Mint 23-module SHA-256 baseline (7 November 2026)

This is a **snapshot of the user's existing independent modules**, not a universal
release reference. The file [Mint SO preflight SHA-256 manifest](FILESYSTEMS_MINT_MODULE_BASELINE_2026-11-07.sha256)
can be checked later using `sha256sum -c` on the same machine. Each
line came from the user's read-only preflight and covers a unique .so. Do not
assume source or APT package upgrades are permitted to alter these values.

## Re-entry procedure for the next working session

1. Read this document and the corresponding open GitHub issue **before any source modification**.
2. Query current `main`, latest published release and full CI conclusions for `Filesystems`, `Filesystem-FFS`, `Filesystem-exFAT` (and the specific next format being extracted). Source commits and immutable published tags are separate facts.
3. Choose one unchecked task, implement it directly on `main` with a non-force fast-forward commit, and run relevant independent media/build/packaging checks. Never push provider Git submodules back into central.
4. Update this handover and the issue's checklist with the exact SHA and verification results, including failures. Do not mark a task complete based only on a proposed commit or an in-progress workflow.
5. If time is short, leave all unverified functionality untouched; the latest successful GitHub commit and release is the safe recovery point.

**Immediate human action needed: none.** Published independent FFS/exFAT artifacts exist and their isolated dpkg coexistence passes. Continue with real installed-GUI acceptance and remaining FAT format tests. Do not remove any more source before its independent owner is qualified.

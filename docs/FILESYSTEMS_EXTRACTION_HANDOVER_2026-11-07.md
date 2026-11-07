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
| [Filesystems](https://github.com/Infiltrator-Projects/Filesystems) | `d6bf9d4ef85dfcd29b1defc6528dba3ac7803991` | `0.5.112` | Source-retirement guard and temporary manager compile PASS. **NOT a full installation/UI functional qualification; 0.5.112 is not published.** Last published Filesystems release is `v0.5.110`. |
| [Filesystem-FFS](https://github.com/Infiltrator-Projects/Filesystem-FFS) | `dc62ac594c6959b6fd03093a30fb9c6e0fdc9b1c` | `1.0.7` | [`v1.0.7`](https://github.com/Infiltrator-Projects/Filesystem-FFS/releases/tag/v1.0.7) **published**, independent FFS SO and source-RUN/package workflow PASS, includes `ffs-linux-6.12.107.ko`, native `.run`, `.deb`. |
| [Filesystem-exFAT](https://github.com/Infiltrator-Projects/Filesystem-exFAT) | `02d054202e4bbb2b3f1da36391019b64975842cb` | `0.1.10` | [`v0.1.10`](https://github.com/Infiltrator-Projects/Filesystem-exFAT/releases/tag/v0.1.10) **published**, standalone SO, defrag, kernel/QEMU and package workflow PASS; assets include `exFAT.so`, `exfat-linux-6.12.107.ko`, `.run`, `.deb`. |

GitHub commits are durable recovery points; do not confuse a successful *temporary manager compilation* with a functional installed application.

Prior extracted-source commits on Filesystems main:
- [`99f29cb`](https://github.com/Infiltrator-Projects/Filesystems/commit/99f29cb4f8e06db4eb427b42b925ff5c94f9463f): remove 23 filesystem submodule links, central FAT implementation and extracted source placeholders; remove direct builds.
- [`f0e8d6e`](https://github.com/Infiltrator-Projects/Filesystems/commit/f0e8d6ec7aa8c42f67f02b49b297310a46fc25e2): retire obsolete per-format build/test helpers and **stop central Filesystems APT publication**.
- [`df93e6a`](https://github.com/Infiltrator-Projects/Filesystems/commit/df93e6a923d3559b1cf7e593c6bd48f1fed2d7b6): remove `fsinspect` format engine linked to extracted Amiga cores; [boundary + manager build PASS](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42327315107).
- [`d6bf9d4`](https://github.com/Infiltrator-Projects/Filesystems/commit/d6bf9d4ef85dfcd29b1defc6528dba3ac7803991): correct source inventories and flag historical documentation; [boundary + manager build PASS](https://github.com/Infiltrator-Projects/Filesystems/actions/runs/42349589099).

## Known unfinished work — next actions IN ORDER

- [ ] **First fix the central manager's now-stale Install/Remove paths without reintroducing source engines.** `src/probe.cpp` checks removed `/Programs/Filesystems/Helpers/{fat12,fat16,fat32,ofs,ext2,minix}-module-helper` binaries; `src/installer.cpp`, `src/desktop_actions.cpp`, `src/action_policy.cpp`, and `src/main.cpp` still refer to them. Define a provider-independent installation/status contract for independently packaged `Filesystem-*` releases (likely through InfiltratorOS); validate presence and identity against actual installed modules and package ownership. **Do not claim these buttons work just because CMake compiles.** If necessary until integration is implemented, explicitly disable misleading actions rather than silently requesting missing helpers.
- [ ] Add installed-package/UI functional tests on disposable environments that exercise status, install and removal for separate `FFS.so`, `exFAT.so`, and FAT12/16/32 without dependence on central provider source trees. Test no unexpected `.so` replaces another format and no physical-media operations occur.
- [ ] Enumerate the **21** remaining `native/filesystems/*` source-format directories against `data/source-filesystems.txt`, and map each to a specific future standalone owner. Extract only after that standalone repo has the complete best implementation, license/provenance, `.so`, applicable `.ko`, documentation, tests and passing qualification. **Then delete its source from central** and update manifests/build/tests.
- [ ] Keep `data/native-filesystems.tsv` consistent with remaining centrally-owned module code (currently BFS and UDF), but do not assume a .ko is qualified simply because source exists.
- [ ] Audit historical `docs/*` and old open issues for extracted formats. Move authoritative knowledge/bugs to the corresponding standalone repo; keep traceable links rather than losing historical design requirements. Historical issue numbers #61, #45 and OFS #35/#33/#31/#29/#27 still refer to past central work.
- [ ] Coordinate InfiltratorOS, Defragmenter, Files, BOOT, Setup and kernel consumers using the agreed per-filesystem ABI. A per-format `.ko` or `.efi` is currently a separate qualified compatibility path; a Linux userspace `.so` cannot be executed directly by an ordinary kernel/UEFI loader.
- [ ] **Only at the end:** establish and pass end-to-end installer, upgrade, removal, generic loader and recovery tests; then decide when central `Filesystems` can be retired. Do not delete its repository or make its old release disappear prematurely.

## Re-entry procedure for the next working session

1. Read this document and the corresponding open GitHub issue **before any source modification**.
2. Query current `main`, latest published release and full CI conclusions for `Filesystems`, `Filesystem-FFS`, `Filesystem-exFAT` (and the specific next format being extracted). Source commits and immutable published tags are separate facts.
3. Choose one unchecked task, implement it directly on `main` with a non-force fast-forward commit, and run relevant independent media/build/packaging checks. Never push provider Git submodules back into central.
4. Update this handover and the issue's checklist with the exact SHA and verification results, including failures. Do not mark a task complete based only on a proposed commit or an in-progress workflow.
5. If time is short, leave all unverified functionality untouched; the latest successful GitHub commit and release is the safe recovery point.

**Immediate human action needed: none.** The published independent FFS/exFAT artifacts exist. Continue with central manager reconciliation, not more destructive filesystem removal.

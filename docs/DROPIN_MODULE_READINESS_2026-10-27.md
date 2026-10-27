# Standalone filesystem .so qualification ledger — 27 October 2026

This ledger intentionally distinguishes actual standalone shared objects from
filesystem catalogue identities, Linux kernel `.ko` modules, EFI drivers,
copied source, install scripts and formatting tools. **Code existence is not
capability qualification.** A working `.ko`, a formatter or a firmware reader
does not prove the corresponding standalone `.so` feature exists.

The Filesystems catalogue contains **41 on-media format identities**.
Classic FAT12/FAT16/FAT32 share the *one* existing classic FAT catalogue
entry but compile as **three different independent binaries**, not one
renamed library. The six .so binaries below cover **four** catalogue entries:
classic FAT, OFS, EXT2 and SCO BFS. Consequently **37 catalogue entries**
still have no standalone drop-in module here. This is a *format coverage*
count, not `41 minus 6`.

| Standalone binary | V1 read scope | Optional existing-data overwrites | Unimplemented major functions |
| --- | --- | --- | --- |
| `FAT12.so` | 8.3 root files, FAT12 chain reads/fragment counts | Proposed V2 (PR #51) | Full RW, create/delete, allocate/resize, nested/LFN, formatter, defrag |
| `FAT16.so` | 8.3 root files, FAT16 chain reads/fragment counts | Proposed V2 (PR #51) | Full RW, create/delete, allocate/resize, nested/LFN, formatter, defrag |
| `FAT32.so` | Cluster-chained root, FAT32 chain reads/fragment counts | Proposed V2 (PR #51), default active FAT only | Full RW, create/delete, allocate/resize, nested/LFN, formatter, defrag |
| `OFS.so` | 512-byte-block AmigaDOS DOS0/2/4 directory lookup, enumerate, read | None | Full RW, formatter, defrag, larger OFS blocks |
| `EXT2.so` | Existing canonical EXT2 engine, inode lookup/read | Limited existing-block overwrite (draft PR #49) | Full RW, create/delete, append, allocation, formatter, defrag |
| `BFS.so` | SCO BFS inode, root directory and contiguous-file reads | Limited existing-byte overwrite (draft PR #50) | Full RW, create/delete, append, allocation, formatter, defrag |

**Status as at this ledger's date:**

- The pre-existing four FAT12/FAT16/FAT32/OFS binaries have independent read
  tests. Their V1 APIs do not advertise writing.
- EXT2 and SCO BFS are additional independent **development candidates**,
  not proof of installed or released binaries. Their code, compile targets,
  source-packaged helpers and tests are in stacked drafts.
- The new classic FAT V2 APIs preserve V1 symbols and the unchanged V1 binary
  structure; a V1 consumer is still read-only. A V2 consumer must deliberately
  opt into the constrained write callback.
- The native design places selected binaries directly in
  `/System/Filesystems/`; application sources and helpers belong under
  `/Programs/Filesystems/`. Neither a package nor these drafts should
  silently replace existing host filesystem drivers.
- No full read/write standalone `.so` has been qualified. Limited
  in-place data overwrites are **not** directory or metadata mutation,
  transactional writeback, boot qualification or release readiness.
- CI uses disposable images and runners. Physical device and Boot
  interoperability are separate gates before enabling affected paths.

For source inventory and catalogue coverage see
[`SOURCE_BASELINE_STATUS.md`](SOURCE_BASELINE_STATUS.md) and
[`FILESYSTEMS_MATRIX.md`](FILESYSTEMS_MATRIX.md). This is a dated
capability snapshot rather than a substitute for reproducible CI results.

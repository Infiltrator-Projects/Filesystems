# FAT12 / FAT16 / FAT32 Design and Ownership

> **Working source (23 October 2026).** This filesystem's `linux/` directory contains the actual copied Linux implementation source, preserved with its original attribution and available for direct modification. It is not a replaceable research-only reference tree. The old independently-rewrite-first policy and any old directory-layout/refresh language below are historical, not a restriction on editing this working baseline. Only explicitly qualified drivers appear in `data/native-filesystems.tsv`.


## Classification

This directory owns the classic FAT filesystem family: FAT12, FAT16 and FAT32,
including DOS short-name and VFAT long-filename namespace behaviour. Those are
format/namespace variants of one filesystem family, not three unrelated host
implementations.

## Current state

FAT is currently **imported-source development state**. The pinned Linux FAT/VFAT/MS-DOS
implementation is preserved under `linux/` with upstream licensing
and provenance. It is not project-authored Filesystems code.

## Current directory contract

While FAT remains in imported-source development state, the filesystem root contains only
`DESIGN.md` and `linux/`, with the pinned upstream Linux FAT/MS-DOS/VFAT
implementation under `linux/`. The future `core/`, `linux/`,
`windows/` and `userspace/` directories appear only with independently
authored implementation; `kernel/` is not a valid alternate source tree.

CI enforces this boundary and rejects accidental production linkage of the
imported reference source.

## Target layout

```text
native/filesystems/fat/
  DESIGN.md
  core/                 canonical FAT12/16/32 + directory/name semantics
  linux/                thin Linux VFS/module adapter
  windows/              thin Windows IFS/WDK adapter
  userspace/            optional adapter over the same core
  linux/      pinned upstream Linux evidence
```

## Canonical ownership

The core owns BPB/boot-sector and geometry validation, FAT12/16/32 entry
encoding, cluster-chain traversal/allocation, FSInfo handling where applicable,
directory entries, volume labels, timestamps/attributes, short-name rules,
VFAT long-filename entry chains/checksums and exact mutation/recovery rules.

Linux NLS interfaces, VFS objects, page/buffer cache and mount/module lifetime
remain adapter concerns.

## Namespace rule

Linux's separate `namei_msdos.c` and `namei_vfat.c` files are reference
translation units. The project core should model short-name-only and VFAT
long-name behaviour as explicit namespace policy over the same FAT engine,
without creating competing FAT implementations.

## Promotion sequence

Start with host-neutral boot/FAT/directory parsing across FAT12/16/32 and
independent media fixtures. Add mutation only after cluster allocation,
directory entry publication, FSInfo updates and crash/recovery expectations are
explicitly qualified.

## Failure policy

Impossible geometry, inconsistent FAT copies, loops/out-of-range cluster
chains, malformed LFN sequences and contradictory free-space metadata must fail
closed or be surfaced as corruption according to the operation.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

# FAT12 / FAT16 / FAT32 Design and Ownership

> **Working source (23 October 2026).** This filesystem's `linux/` directory contains the actual copied Linux implementation source, preserved with its original attribution and available for direct modification. It is not a replaceable research-only reference tree. The old independently-rewrite-first policy and any old directory-layout/refresh language below are historical, not a restriction on editing this working baseline. Only explicitly qualified drivers appear in `data/native-filesystems.tsv`.


## Classification

This directory owns the classic FAT filesystem family: FAT12, FAT16 and FAT32,
including DOS short-name and VFAT long-filename namespace behaviour. Those are
format/namespace variants of one filesystem family, not three unrelated host
implementations.

## Current state

The FAT16 consolidation now includes copied working Defragmenter FAT geometry, directory and Test Media creator code under `consolidation/defragmenter/`. The compiled `FAT16.so` includes a backward-compatible v2 directory reader, using a freestanding port of Defragmenter's VFAT LFN and cluster enumeration logic. The originals are staged for subsequent mutation/formatter integration; they are not yet built into the module. The original Linux FAT adapter remains in **imported-source development state**. The first independent FAT16 module is a separate experimental read-only module under `module/`, compiled on demand to `FAT16.so`. The pinned Linux FAT/VFAT/MS-DOS
implementation is preserved under `linux/` with upstream licensing
and provenance. It is not project-authored Filesystems code.

## Current source layout and compilation boundary

```text
native/filesystems/fat/
  DESIGN.md
  linux/                    editable original Linux FAT/VFAT source
  module/
    fat16_module.c           compiled experimental v1 reader
    fat16_namespace.c        compiled experimental v2 directory/LFN port
    fat16_module.h           v1/v2 reader ABI declarations
  consolidation/
    README.md
    defragmenter/            actual copied FAT engine, writer and creator
```

The original FAT/VFAT Linux implementation in `linux/` can be directly
developed but is not currently built into this experimental `.so`. The
Defragmenter copies are also editable working source, retained under their
original attribution, **not** automatically compiled into `FAT16.so`.
The staged mutation, formatting, relocation, journaling and recovery
implementations need host-I/O separation and tests before activation.

Future responsibility-based `core/`, `linux/`, `windows/` and firmware
adapter boundaries are introduced as they are implemented and qualified;
they are not evidence that those adapters already exist or run. `kernel/`
is not a separate alternate format engine. One family source can produce
three separately installable FAT12/FAT16/FAT32 modules, but only FAT16's
experimental selector currently exists in the 41-format catalogue.


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

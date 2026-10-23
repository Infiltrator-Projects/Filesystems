# UDF Filesystem Design and Ownership

> **Working source (23 October 2026).** This filesystem's `linux/` directory contains the actual copied Linux implementation source, preserved with its original attribution and available for direct modification. It is not a replaceable research-only reference tree. The old independently-rewrite-first policy and any old directory-layout/refresh language below are historical, not a restriction on editing this working baseline. Only explicitly qualified drivers appear in `data/native-filesystems.tsv`.


## Classification

UDF is a conventional local filesystem format based on ECMA-167 and used on
optical media, removable storage and filesystem images. Filesystems
therefore applies the standard canonical-engine model.

## Current implementation state

UDF is currently **imported-source development state**.

There is no project-authored canonical UDF engine and no Filesystems UDF
native module claimed by this directory. The source under `linux/`
is pinned upstream Linux UDF source retained as format, interoperability and
failure-behaviour evidence.

The imported files keep their upstream licences, copyrights, filenames and
translation-unit boundaries. The former top-level `kernel/` staging path was
misleading because it made unchanged upstream Linux source appear to be
project-native implementation.

## Current directory layout

```text
native/filesystems/udf/
  DESIGN.md
  reference/
    linux/               pinned upstream Linux UDF source, reference only
```

When an independent implementation begins:

```text
native/filesystems/udf/
  DESIGN.md
  core/                  canonical ECMA-167/UDF format/filesystem semantics
  linux/                 thin Linux VFS/module adapter
  windows/               thin Windows adapter when implemented
  userspace/             optional image/tool adapter over the same core
  linux/       preserved upstream evidence
```

## Canonical ownership

A future `core/` owns UDF-defined behaviour including:

- volume recognition and descriptor-sequence validation;
- ECMA-167 descriptor tag/checksum/CRC rules;
- logical-volume and partition-map interpretation;
- allocation descriptors and file-entry/extended-file-entry semantics;
- file identifier descriptors, directory and Unicode/name rules;
- VAT/sparable/metadata-partition semantics when explicitly supported;
- inode/file mapping, symlink and timestamp representation;
- filesystem allocation and persistent mutation rules;
- corruption, range and feature validation.

Linux VFS objects, block/page-cache APIs, credentials, locking and
mount/module lifetime are platform-adapter concerns.

## File-boundary rule

Imported Linux units such as `super.c`, `inode.c`, `partition.c`,
`directory.c`, `balloc.c`, `unicode.c` and the ECMA/UDF headers remain
unchanged beneath `linux/`.

Project-authored source must be cut around portable UDF responsibilities rather
than by renaming or recommenting those reference translation units.

## Promotion sequence

1. define the exact UDF revisions, partition maps and media classes supported;
2. implement bounded host-neutral descriptor/tag/volume parsing;
3. implement read-only namespace, file and allocation traversal on independent
   media/images;
4. qualify Unicode/name, VAT/sparable/metadata and malformed-media cases that
   are claimed;
5. add a thin Linux adapter and optional userspace inspection over the same core;
6. add mutation only with exact allocation/publication/recovery rules and
   destructive qualification;
7. add a Windows adapter against the same canonical engine when appropriate.

## Failure policy

Invalid descriptor tags/CRCs, contradictory volume sequences, out-of-range
partition/allocation references, malformed directory records and unsupported
partition-map or revision states must fail closed. Optical/removable media
peculiarities must remain explicit rather than guessed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is reference evidence only. UDF becomes a Filesystems
first-party implementation only when its canonical engine and applicable
platform adapter have independent implementation and qualification evidence.

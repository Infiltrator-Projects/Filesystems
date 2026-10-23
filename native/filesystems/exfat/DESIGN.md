# exFAT Design and Ownership

> **Working source (23 October 2026).** This filesystem's `linux/` directory contains the actual copied Linux implementation source, preserved with its original attribution and available for direct modification. It is not a replaceable research-only reference tree. The old independently-rewrite-first policy and any old directory-layout/refresh language below are historical, not a restriction on editing this working baseline. Only explicitly qualified drivers appear in `data/native-filesystems.tsv`.


## Classification

exFAT is a conventional local removable-media filesystem. Filesystems
must implement its format semantics once and share them across host adapters.

## Current state

exFAT is currently **imported-source development state**. The pinned upstream Linux
implementation is preserved under `linux/` with original licensing,
copyright and filenames. It is not project-authored Filesystems code.

## Current directory contract

While exFAT remains in imported-source development state, the filesystem root contains only
`DESIGN.md` and `linux/`, with the pinned upstream Linux implementation
under `linux/`. The future `core/`, `linux/`, `windows/` and
`userspace/` directories appear only with independently authored implementation;
`kernel/` is not a valid alternate source tree.

CI enforces this boundary and rejects accidental production linkage of the
imported reference source.

## Target layout

```text
native/filesystems/exfat/
  DESIGN.md
  core/                 canonical exFAT format/filesystem semantics
  linux/                thin Linux VFS/module adapter
  windows/              thin Windows IFS/WDK adapter
  userspace/            optional userspace adapter over the same core
  linux/      pinned upstream Linux evidence
```

`exfat-fuse` is an alternate provider choice for the same exFAT format, not a
separate catalogue identity or a second exFAT implementation.

## Canonical ownership

The core owns exFAT-defined boot-region/geometry validation, FAT and allocation
bitmap semantics, cluster chains, directory-entry sets, stream/name entries,
upcase-table/name comparison, checksums, timestamps/attributes, file mapping,
allocation and exact mutation/recovery rules.

Linux VFS objects, NLS APIs, page cache and module lifetime stay in the Linux
adapter. Windows IFS objects and platform security/attribute presentation stay
in the Windows adapter.

## Promotion sequence

Begin with bounded format validation and read-only directory/file access,
qualified against independently produced exFAT media and malformed fixtures.
Then implement allocation/mutation with explicit transaction/recovery and
destructive testing. All adapters consume the same core.

## Failure policy

Invalid boot checksums, impossible geometry, contradictory FAT/bitmap state,
bad directory-entry sets, invalid cluster chains and unsupported features fail
closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

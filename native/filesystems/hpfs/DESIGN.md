# HPFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

HPFS (High Performance File System) is a conventional local block filesystem
used by OS/2 and related systems. Filesystems therefore applies the
normal one-canonical-filesystem architecture.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied Linux implementation includes allocation-map, anode, dnode and directory handling.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/hpfs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## Canonical-core ownership

The future `core/` owns HPFS-defined semantics that are independent of the
host operating system, including:

- boot/superblock and spare-block decoding and validation;
- sector, band and allocation-bitmap geometry;
- fnode and anode structures and file block mapping;
- dnode directory B-tree structure, ordering and traversal;
- filename, codepage and case/collation rules defined by HPFS;
- extended attributes and filesystem metadata records;
- free-space accounting and allocation;
- hotfix/remapping state where supported by the format;
- timestamps, attributes and object metadata;
- corruption/range validation;
- exact mutation, ordering and recovery semantics for any supported writer.

Linux VFS objects, buffer/page-cache integration, mount/module lifetime,
credentials and Linux error translation belong in `linux/`.

Windows IFS/WDK objects, IRPs, VCB/FCB/CCB lifetime, Cache Manager integration
and NTSTATUS translation belong in `windows/`.

## File-boundary rule

The original files may be edited directly. Changing a filename or translating a body does not by itself change authorship; only actual independent replacement qualifies as project-authored source. Keep working functionality and attribution while moving any genuinely shared format rules to a canonical engine.


Reference filenames such as `alloc.c`, `anode.c`, `dnode.c`, `map.c`,
`namei.c` and `super.c` remain unchanged under `linux/` so
their provenance is explicit.

Project-authored source must be cut around Filesystems responsibilities
and portable filesystem invariants. Moving, renaming, merging or recommenting
copied Linux source is never evidence that the implementation has been
rewritten.

## Promotion sequence

The milestones below describe the remaining format-specific engineering and acceptance tests. They can be implemented by adapting and qualifying the existing source; they do not require a speculative replacement before editing can begin. A separately pursued project-authored rewrite must preserve original provenance until the relevant code has actually been replaced.


1. define supported HPFS versions, codepages and media geometry;
2. implement bounded host-neutral superblock/spare-block/fnode/anode decoding;
3. implement allocation and directory-tree traversal with independent fixtures;
4. qualify names, codepage/case behaviour and extended attributes;
5. test malformed/corrupt media and out-of-range structures;
6. add a thin read-only Linux adapter over the canonical core;
7. add mutation only after allocation/update/recovery rules are explicitly
   documented and destructively qualified;
8. add a Windows adapter against the same core when ready;
9. remove inherited provenance only from bodies that have actually been
   independently replaced.

## Safety and failure behaviour

Invalid geometry, contradictory allocation maps, malformed fnode/anode chains,
invalid dnode ordering, out-of-range sectors, unsupported codepage/collation
state and unsafe recovery state must fail closed.

The presence of write paths in an external reference implementation is not
evidence that Filesystems has qualified HPFS writes.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

HPFS remains unqualified imported implementation source until a canonical project-authored engine
and at least one adapter have independent implementation and qualification
evidence.

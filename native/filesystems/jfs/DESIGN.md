# JFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

JFS is a conventional local journalled block filesystem. Filesystems
therefore applies the one-canonical-filesystem architecture: format and
filesystem semantics belong in a host-neutral core; operating-system code is an
adapter around that core.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied source includes JFS metadata, directory and transaction implementation.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/jfs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## Canonical-core ownership

The future core owns JFS-defined semantics independent of the host OS,
including:

- superblock and aggregate/fileset geometry;
- inode allocation groups and inode-map semantics;
- block allocation maps and extent descriptors;
- directory B+tree and name/record rules;
- file extent mapping;
- journal/log record format, transaction and replay invariants;
- extended attributes and filesystem metadata records;
- free-space/accounting rules;
- corruption/range validation;
- exact mutation and crash-recovery semantics.

Linux VFS objects, page/buffer cache, block-device plumbing, workqueues,
credentials, quota framework glue and module lifetime belong in the Linux
adapter.

## File-boundary rule

The original files may be edited directly. Changing a filename or translating a body does not by itself change authorship; only actual independent replacement qualifies as project-authored source. Keep working functionality and attribution while moving any genuinely shared format rules to a canonical engine.


Upstream Linux filenames remain unchanged beneath `linux/`.
Project-owned source must be organised around Filesystems
responsibilities and host-neutral JFS invariants, not copied or mechanically
renamed from Linux.

Moving, renaming, merging or recommenting external source is never evidence that
the implementation has crossed the project-authorship boundary.

## Promotion sequence

The milestones below describe the remaining format-specific engineering and acceptance tests. They can be implemented by adapting and qualifying the existing source; they do not require a speculative replacement before editing can begin. A separately pursued project-authored rewrite must preserve original provenance until the relevant code has actually been replaced.


1. document supported on-disk format and journal variants;
2. implement bounded superblock, inode-map, allocation-map and extent decoding;
3. implement read-only directory/file traversal with independent fixtures;
4. implement and test journal/log decoding and replay invariants;
5. qualify malformed media and crash-state handling;
6. add a thin Linux adapter over the canonical core;
7. add mutation only after transaction/recovery ordering is independently
   implemented and destructively qualified;
8. add a Windows adapter against the same core when ready.

## Safety and failure behaviour

Invalid geometry, contradictory allocation/inode maps, malformed B+tree nodes,
out-of-range extents, bad journal records and uncertain replay state must fail
closed.

Write support must not be inferred from the capabilities of the reference
driver.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable, unqualified original implementation source. JFS is not a Filesystems
native implementation until the canonical core and applicable adapter are
independently implemented and qualified.

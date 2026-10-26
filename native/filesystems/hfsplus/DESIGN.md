# HFS+ / HFSX Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

HFS+ (Mac OS Extended) and HFSX are conventional local block filesystems.
HFSX is the case-sensitive variant of the same filesystem family; it is not a
second independent filesystem implementation.

Filesystems therefore applies the canonical local-filesystem
architecture:

```text
                 hfsplus/core/
          canonical HFS+/HFSX semantics
               /                \
              /                  \
     hfsplus/linux/         hfsplus/windows/
       thin VFS                thin IFS
       adapter                 adapter
```

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied HFS+ implementation includes B-trees, catalogues, extents and journalling-related support; HFSX ordering and writes require separate qualification.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/hfsplus/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## Canonical-core ownership

The future canonical core owns HFS+-defined and HFSX-defined behaviour that is
independent of the host operating system, including:

- volume-header decoding, validation and feature/state policy;
- allocation bitmap and block-allocation semantics;
- catalog-file B-tree keys, records, traversal and mutation;
- extents-overflow B-tree and fork extent mapping;
- attribute-file B-tree and extended-attribute records;
- file and directory CNID/object identity rules;
- data and resource fork representation;
- Unicode name encoding, normalization/comparison and catalog key ordering;
- HFSX case-sensitive/case-folding policy as defined by the format;
- timestamps, Finder metadata and filesystem-defined flags;
- hard-link and symbolic-link representation;
- wrapper/embedded HFS volume handling where required for HFS+ discovery;
- journal state and recovery semantics only for variants explicitly supported;
- corruption, bounds and structural validation;
- filesystem-defined mutation and crash-consistency rules.

Linux VFS objects, dentries/inodes, page cache, buffer heads, kernel Unicode
helpers, credentials, mount/module lifetime and Linux error translation belong
in the Linux adapter.

Windows IRPs, VCB/FCB/CCB lifetime, Cache Manager integration, native security
presentation and NTSTATUS translation belong in the Windows adapter.

## HFS+ and HFSX rule

HFS+ and HFSX share one canonical implementation.

Case-sensitive HFSX behaviour and any HFSX-specific catalog-key rules must be
explicit negotiated filesystem state in the canonical core. They must not be
implemented as a second host-specific filesystem engine.

Classic HFS remains a separate filesystem under
`native/filesystems/hfs/`; shared ancestry is not permission to merge HFS and
HFS+ implementations.

## File-boundary rule

The original files may be edited directly. Changing a filename or translating a body does not by itself change authorship; only actual independent replacement qualifies as project-authored source. Keep working functionality and attribution while moving any genuinely shared format rules to a canonical engine.


The upstream reference filenames such as `btree.c`, `catalog.c`,
`extents.c`, `inode.c`, `super.c`, `unicode.c` and `xattr.c` remain
unchanged under `linux/` so their provenance is obvious.

Project-authored HFS+ files must be cut around Filesystems
responsibilities and portable invariants. Copying, renaming, merging,
recommenting or moving an upstream file is never evidence that its
implementation has been rewritten.

## Promotion sequence

The milestones below describe the remaining format-specific engineering and acceptance tests. They can be implemented by adapting and qualifying the existing source; they do not require a speculative replacement before editing can begin. A separately pursued project-authored rewrite must preserve original provenance until the relevant code has actually been replaced.


1. define the supported HFS+/HFSX media variants and feature/journal limits;
2. implement bounded host-neutral volume-header and fork/extents decoding;
3. implement catalog/extents/attribute B-tree traversal with independent
   fixtures;
4. implement Unicode/name comparison rules and HFSX policy in the canonical
   core;
5. qualify read-only traversal against independently produced HFS+/HFSX media
   and malformed/corrupt fixtures;
6. add a thin Linux adapter over the canonical core;
7. add mutation only after allocation, B-tree update, journal/recovery and
   crash-consistency contracts are explicitly defined and destructively
   qualified;
8. add a Windows adapter against the same canonical engine when ready;
9. remove inherited provenance only from implementation bodies that have
   actually been independently replaced.

## Safety and failure behaviour

Unknown or unsupported format features, invalid B-tree nodes, impossible
extent/fork geometry, allocation-bitmap contradictions, invalid CNIDs,
malformed Unicode/catalog keys, corrupt attribute records and unsafe journal
state must fail closed.

Write support must never be inferred merely because the reference Linux driver
contains write paths.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

A Filesystems HFS+ implementation is not claimed until a canonical core
and at least one adapter have independent implementation and qualification
evidence. the current editable source is not yet independently qualified as a Filesystems native provider.

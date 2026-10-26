# Btrfs Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

Btrfs is a conventional local copy-on-write filesystem with checksums,
snapshots/subvolumes and multi-device capabilities. It belongs in the normal
canonical-engine architecture.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied Linux implementation includes chunk/device mapping, B-trees, COW transactions and snapshot/subvolume mechanisms.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/btrfs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## Target integration

Retain the editable `linux/` source and its original provenance. Separate filesystem-defined semantics into one canonical core as cross-platform consumers are implemented, without discarding existing functional behaviour. A Linux `.ko`, Windows `.sys`, EFI reader and drop-in application `.so` have distinct ABI/build/qualification contracts. 

## Canonical ownership

The future host-neutral core owns Btrfs-defined structures and behaviour:
superblocks/chunk trees, key and B-tree semantics, extents/backreferences,
checksums, copy-on-write transactions, subvolumes/snapshots, device/chunk
mapping, allocation, compression format handling, namespace/metadata rules and
filesystem-defined recovery/repair invariants.

Linux folios, bios, workqueues, VFS objects, kernel locking and mount/module
lifetime remain Linux-adapter concerns.

## File-boundary rule

The original files may be edited directly. Changing a filename or translating a body does not by itself change authorship; only actual independent replacement qualifies as project-authored source. Keep working functionality and attribution while moving any genuinely shared format rules to a canonical engine.


Upstream files such as `ctree.c`, `inode.c`, `extent-tree.c`,
`tree-log.c`, `volumes.c` and related headers remain inherited source filenames until responsibility-based refactoring is justified.
Project-authored files should be organised around Filesystems ownership and
portable invariants rather than copied/renamed from Linux.

## Promotion sequence

The next integration and qualification phase begins with bounded format/feature decoding and read-only tree,
chunk/device and extent traversal, qualified against independent images and
malformed-media cases. Mutation, snapshots/subvolumes, multi-device writes and
repair require separate durability/recovery qualification before they can be
claimed.

A Linux adapter and any future Windows adapter must consume the same core.

## Failure policy

Unsupported incompatibility features, invalid tree keys/levels, checksum
failures, contradictory chunk/device maps, out-of-range extents and unsafe
transaction/log state fail closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

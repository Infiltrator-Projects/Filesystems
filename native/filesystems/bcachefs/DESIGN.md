# Bcachefs Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

Bcachefs is a conventional local copy-on-write block filesystem. Filesystem
Support therefore applies the canonical local-filesystem architecture:

```text
                 bcachefs/core/
          canonical Bcachefs semantics
               /              \
              /                \
     bcachefs/linux/       bcachefs/windows/
       thin VFS              thin IFS
       adapter               adapter
```

The existing Linux implementation is valuable semantic/reference evidence, but
its Linux-kernel translation-unit boundaries are not the project architecture.

## Canonical-core ownership

When Bcachefs is independently rewritten, filesystem-defined behaviour belongs
in `core/`, including as applicable:

- persistent superblock/format and feature interpretation;
- bkey/B-tree key ordering, traversal and update semantics;
- extent, allocation and free-space semantics;
- copy-on-write and backpointer/reference rules;
- journal/recovery and checkpoint ordering;
- device and replica placement semantics that are format-defined;
- checksums, compression/encryption format interpretation where supported;
- inode, directory, xattr and subvolume/snapshot semantics;
- corruption/range validation and filesystem-defined repair invariants.

Linux workqueues, folios, block-device objects, kernel locks, VFS inode/file
objects, netlink/debug plumbing and other host mechanisms are not canonical
filesystem semantics.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. Its copied implementation includes allocation, B-trees, journalling, recovery, snapshots and data I/O; their presence is source availability, not project runtime qualification.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/bcachefs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## File-boundary rule

The original files may be edited directly. Changing a filename or translating a body does not by itself change authorship; only actual independent replacement qualifies as project-authored source. Keep working functionality and attribution while moving any genuinely shared format rules to a canonical engine.


Upstream filenames such as `btree_*.c`, `alloc_*.c`, `journal_*.c` and
kernel-specific support files are retained unchanged in `linux/` so
their provenance is obvious.

Project-authored files must be cut around Filesystems responsibilities
and host-neutral ownership, not mechanically copied or renamed from Linux.
Moving an upstream file is never evidence that it has crossed the authorship
boundary.

## Promotion sequence

The milestones below describe the remaining format-specific engineering and acceptance tests. They can be implemented by adapting and qualifying the existing source; they do not require a speculative replacement before editing can begin. A separately pursued project-authored rewrite must preserve original provenance until the relevant code has actually been replaced.


1. document the supported on-disk format and feature set;
2. establish bounded host-neutral format decoding and feature negotiation;
3. implement read-only B-tree/key/extents traversal with independent fixtures;
4. qualify malformed/corrupt media failure behaviour;
5. add a thin Linux adapter over the canonical engine;
6. implement mutation only with explicit journal/recovery contracts and
   destructive qualification;
7. add Windows support against the same engine when the canonical contracts are
   mature;
8. retire reference reliance subsystem-by-subsystem only after independent
   replacement and tests.

## Failure policy

Unknown incompatible features, contradictory device/replica state, invalid
B-tree/key geometry, bad checksums, out-of-range extents and unrecoverable
journal state must fail closed. Multi-device and repair operations require
stronger evidence than a successful mount.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

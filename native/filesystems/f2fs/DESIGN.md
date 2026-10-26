# F2FS Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

F2FS is a conventional local flash-oriented filesystem with checkpoint,
segment, node and garbage-collection semantics. Filesystems must
implement those semantics once in a host-neutral core.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied Linux implementation includes checkpoint/node/segment mechanisms and native F2FS mount code.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/f2fs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## Target integration

Retain the editable `linux/` source and its original provenance. Separate filesystem-defined semantics into one canonical core as cross-platform consumers are implemented, without discarding existing functional behaviour. A Linux `.ko`, Windows `.sys`, EFI reader and drop-in application `.so` have distinct ABI/build/qualification contracts. 

## Canonical ownership

The core owns F2FS-defined superblock/checkpoint validation, segment/NAT/SIT
state, node addressing, allocation, summary blocks, recovery, directory/hash
rules, inline data/xattrs, compression-format interpretation, zoned/flash
geometry policy where format-defined, and exact mutation/recovery invariants.

Linux VFS objects, bios/folios, writeback, workqueues, sysfs/debug plumbing and
kernel-specific crypto/verity hooks are adapter mechanisms.

## Promotion sequence

Begin with bounded host-neutral format/checkpoint decoding and read-only node,
directory and data traversal. Qualify independently manufactured images and
malformed checkpoint/segment metadata. Add writes, GC, checkpoint publication
and recovery only after explicit durability/destructive qualification.

## Failure policy

Invalid checkpoint packs, contradictory NAT/SIT/segment state, bad node
addresses, unsupported incompat features and unsafe recovery state fail closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

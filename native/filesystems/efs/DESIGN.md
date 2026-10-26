# SGI EFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

EFS is Silicon Graphics' legacy Extent File System, predating XFS. It is a
conventional local block filesystem and the present catalogue exposes it as
read-only.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied upstream driver provides EFS decoding and read-oriented Linux integration.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/efs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## Target integration

Retain the editable `linux/` source and its original provenance. Separate filesystem-defined semantics into one canonical core as cross-platform consumers are implemented, without discarding existing functional behaviour. A Linux `.ko`, Windows `.sys`, EFI reader and drop-in application `.so` have distinct ABI/build/qualification contracts. 

## Canonical ownership

The core owns EFS-defined superblock/geometry validation, inode layout,
extent/block mapping, directory/name semantics, symlink interpretation and
corruption/range checks.

Linux VFS inode/file/dentry objects, buffer cache and mount/module lifecycle
belong only in the Linux adapter.

## Read-only rule

The project must not infer write semantics from the existence of inode/file
operations in another implementation. EFS remains read-only until exact
allocation/update/recovery semantics are independently documented and a
deliberate write milestone is admitted.

## Promotion sequence

Implement bounded host-neutral parsing first, qualify against independent EFS
images and malformed fixtures, then add a thin read-only host adapter.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

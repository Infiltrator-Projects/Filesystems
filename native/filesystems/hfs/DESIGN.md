# Classic HFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

HFS is the classic Macintosh Hierarchical File System. It is a conventional
local block filesystem and therefore belongs in the canonical-engine
architecture.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied Linux implementation provides classic HFS volume/catalogue/extent reading.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Target integration

Retain the editable `linux/` source and its original provenance. Separate filesystem-defined semantics into one canonical core as cross-platform consumers are implemented, without discarding existing functional behaviour. A Linux `.ko`, Windows `.sys`, EFI reader and drop-in application `.so` have distinct ABI/build/qualification contracts. 

## Canonical ownership

The future core owns HFS master-directory-block and volume geometry, allocation
bitmap, extents overflow and catalog B-tree semantics, file/resource forks,
directory/catalog records, classic Mac metadata/attributes, name translation
policy and exact mutation/recovery invariants.

Linux VFS objects, page/buffer APIs, mount/module lifetime and native permission
presentation remain adapter concerns.

## File-boundary rule

The original files may be edited directly. Changing a filename or translating a body does not by itself change authorship; only actual independent replacement qualifies as project-authored source. Keep working functionality and attribution while moving any genuinely shared format rules to a canonical engine.


Upstream `btree.c`, `catalog.c`, `extent.c`, `inode.c`, `super.c` and
related files retain their original names only in `linux/`.
Project-owned source is cut by portable HFS responsibility, not copied Linux
translation units.

## Promotion sequence

Start read-only with bounded MDB, allocation, catalog and extent parsing against
independently created HFS images and malformed fixtures. Add writes only after
catalog/extents/allocation publication and recovery/verification rules are
explicit and destructively qualified.

## Failure policy

Invalid allocation/catalog/extents B-tree nodes, contradictory volume
geometry, out-of-range extents, malformed names and unsupported HFS states fail
closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

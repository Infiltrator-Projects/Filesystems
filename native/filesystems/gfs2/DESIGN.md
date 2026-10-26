# GFS2 Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

GFS2 is a shared-disk clustered filesystem. Multiple hosts coordinate access to
the same block storage through distributed locking and filesystem journal/
recovery rules.

It is not merely a network namespace, so its permanent architecture includes a
canonical filesystem engine. Cluster lock transport and OS objects remain
adapters around that engine.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied implementation contains Linux GFS2 cluster and locking integration; native shared-disk semantics need separate multi-node tests.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Target integration

Retain the editable `linux/` source and its original provenance. Separate filesystem-defined semantics into one canonical core as cross-platform consumers are implemented, without discarding existing functional behaviour. A Linux `.ko`, Windows `.sys`, EFI reader and drop-in application `.so` have distinct ABI/build/qualification contracts. Cluster locking and coordination remain explicit native integration responsibilities.

## Canonical ownership

The core owns GFS2-defined superblock/resource-group/inode/directory layout,
block mapping and allocation, journals/log replay, quota/statfs semantics,
glock state transitions that are filesystem-defined, and corruption/recovery
invariants.

The Linux VFS/page-cache implementation and Linux DLM calls are adapter
mechanisms. A cluster-lock provider supplies locking/lease operations without
becoming a second implementation of GFS2 metadata or journals.

## Promotion sequence

Begin with bounded host-neutral media decoding and single-node read-only
validation. Add journal/recovery and allocation with independent fixtures.
Clustered mutation is not complete until lock ordering, node failure, journal
recovery and fencing-related failure modes are exercised on real multi-node
test storage.

## Failure policy

Invalid resource groups, lock-state contradictions, bad journal descriptors,
out-of-range block mappings, unsupported format features and uncertain
multi-node recovery state must fail closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

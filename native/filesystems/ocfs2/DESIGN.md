# OCFS2 Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

OCFS2 is a shared-disk clustered filesystem. Multiple hosts may access the same
block storage while coordinating filesystem state through cluster membership,
heartbeat, distributed locking, journals and recovery.

OCFS2 is therefore neither a simple local-only filesystem nor a remote
namespace. Its permanent Filesystems architecture has one canonical
filesystem engine, with host adapters and a separate cluster-coordination
boundary around that engine.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/ocfs2/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The Linux original includes O2CB/DLM clustering in addition to volume, allocation, inode and journal implementation; multi-node operation requires distinct tests.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Target integration and source layout

The existing `linux/` implementation is the starting point for engineering and compatibility, with original attribution retained. Extract format-defined behaviour to one host-neutral canonical core when multiple consumers require it; keep Linux VFS, Windows IFS, firmware and application module execution as independently qualified adapter contracts. The current tree does not imply any Windows/EFI/root deliverable. Preserve O2CB/DLM cluster coordination as an explicit host integration boundary.

## Canonical-core ownership

The future `core/` owns OCFS2-defined behaviour that is independent of a host
kernel object, including as supported:

- superblock, slot-map and system-file interpretation;
- inode, directory and extent-tree semantics;
- allocation groups/suballocation and free-space accounting;
- refcount/reflink and extent-move semantics;
- local/global quota and metadata rules;
- journal format, transaction ordering and filesystem recovery invariants;
- xattr/ACL format semantics;
- resize and filesystem-defined metadata checks;
- filesystem-defined lock/resource identity and ordering requirements;
- corruption, bounds and cross-structure validation.

Linux VFS objects, buffer/page-cache mechanics, block-device APIs, workqueues,
module lifetime and Linux error translation belong in `linux/`.

## Cluster boundary

OCFS2's correctness depends on more than local media structures. Cluster
membership, heartbeat, quorum, distributed locking, node failure and recovery
must have an explicit adapter boundary.

A future `cluster/` layer may expose host-neutral membership, lease/lock,
heartbeat/fencing and recovery coordination mechanisms required by the
canonical filesystem engine. Linux O2CB/DLM objects and configfs/sysfs details
remain platform/reference mechanisms and must not leak into portable
filesystem semantics.

The canonical engine must define what ordering and exclusion it requires; a
cluster provider supplies those mechanisms without becoming a second OCFS2
implementation.

## File-boundary rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


Files below `linux/` retain upstream filenames, SPDX identifiers,
copyrights and source boundaries, including `alloc.c`, `journal.c`,
`dlmglue.c`, the `cluster/` tree and the `dlm/` and `dlmfs/` trees.

Project-authored code must be organised around Filesystems
responsibilities and canonical/adapter ownership. Moving or renaming an
upstream OCFS2 file does not constitute an independent rewrite.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


OCFS2 leaves imported-source development state only through deliberate milestones:

1. document supported on-disk features, cluster model and unsupported states;
2. implement bounded host-neutral superblock/system-file/inode/extent decoding;
3. qualify single-node read-only media against independent images and malformed
   fixtures;
4. implement journal and recovery semantics in the canonical engine;
5. add a thin Linux adapter over that engine;
6. introduce allocation/mutation only with durable recovery and independent
   post-operation verification;
7. implement the cluster adapter and qualify lock ordering, heartbeat,
   membership loss, node failure and recovery on real multi-node storage;
8. add another OS adapter only against the same canonical engine and cluster
   contract.

Clustered write support is not complete merely because a single-node mount can
modify an image.

## Failure policy

Invalid system-file references, impossible allocation metadata, corrupt extent
trees, journal inconsistencies, contradictory slot/lock ownership, lost quorum,
uncertain fencing state and ambiguous node-recovery state must fail closed.

Mutation must not continue when the project cannot prove exclusive or correctly
coordinated access to affected metadata.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

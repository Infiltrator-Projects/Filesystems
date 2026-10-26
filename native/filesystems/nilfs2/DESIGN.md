# NILFS2 Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

NILFS2 is a conventional local log-structured block filesystem with continuous
checkpoint/snapshot and segment-cleaning semantics. Filesystems therefore
applies the one-canonical-filesystem architecture.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/nilfs2/linux/` contains actual Linux filesystem implementation source, which can be developed directly. NILFS2 checkpoint/segment/snapshot and inode/file functions exist in the copied Linux implementation.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/nilfs2/DESIGN.md` alongside `native/filesystems/nilfs2/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical ownership

The future core owns NILFS2-defined host-neutral behaviour including superblock
and geometry validation, segment summaries, checkpoints, DAT/ifile/cpfile/
sufile metadata, inode/directory semantics, block mapping, segment construction,
continuous checkpoint publication, recovery, segment cleaning/GC invariants and
filesystem-defined corruption policy.

Linux VFS objects, page/buffer cache, block-device plumbing, sysfs, workqueues
and module lifetime remain adapter concerns.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


1. define supported NILFS2 format/features and checkpoint rules;
2. implement bounded superblock/checkpoint/segment decoding;
3. implement read-only inode/directory/data traversal against independent media;
4. qualify malformed segment/checkpoint and recovery-state fixtures;
5. add a thin Linux adapter;
6. add mutation, segment construction and cleaner interaction only after
   durability/recovery ordering is explicitly implemented and destructively
   qualified;
7. add Windows against the same canonical core when ready.

## Failure policy

Invalid segment summaries, contradictory checkpoint/DAT state, out-of-range
blocks/inodes, malformed metadata trees and uncertain recovery state must fail
closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable original source, not yet independently qualified for project-native deployment. NILFS2 is not a Filesystems
native implementation until the canonical core and applicable adapter are
independently implemented and qualified.

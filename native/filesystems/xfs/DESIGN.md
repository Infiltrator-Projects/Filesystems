# XFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

XFS is a conventional local journalled extent filesystem. Filesystems
therefore applies the one-canonical-filesystem architecture: persistent XFS
format and filesystem semantics belong in a host-neutral core, while operating
systems expose that core through thin adapters.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/xfs/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The copied XFS Linux source includes libxfs and scrub subsystems; build and repair/format qualification remain separate.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/xfs/DESIGN.md` alongside `native/filesystems/xfs/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical-core ownership

A future `core/` owns XFS-defined, host-neutral behaviour including, for the
explicitly supported feature set:

- superblock, allocation-group and feature-bit interpretation;
- AGF/AGI/AGFL geometry and allocation-group consistency;
- free-space, inode, reverse-mapping and refcount B+tree semantics;
- inode format, forks, extent mapping, unwritten extents and sparse files;
- reflink/shared-extent and copy-on-write persistent semantics;
- directories, names, attributes, remote attributes and symlink representation;
- realtime-device bitmap/summary format where supported;
- persistent quota records and accounting invariants;
- metadata checksums, UUID/owner checks and structural validation;
- log item formats, transaction ordering, log replay and crash recovery;
- persistent online-repair state only where explicitly implemented;
- resize/grow rules that alter persistent filesystem geometry;
- corruption, range and cross-structure consistency validation.

Linux VFS objects, folios/page cache, iomap plumbing, buffer/block-device
interfaces, workqueues, sysfs/procfs/debugfs, Linux credentials, module
lifetime and Linux-specific online-scrub plumbing belong in the Linux adapter
or remain reference mechanisms. Windows IRPs, Cache Manager integration and
native object lifetime belong only in the Windows adapter.

## Reference scrub and libxfs rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


The imported `libxfs/` and `scrub/` subtrees remain part of the upstream
reference snapshot. Their presence is valuable behavioural and repair evidence
but does not make them project-owned canonical code.

Project-authored XFS source must be cut around Filesystems
responsibilities and portable filesystem invariants. Moving, renaming, merging,
splitting or recommenting imported Linux translation units is never evidence
that their implementation has crossed the project-authorship boundary.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


XFS becomes eligible for separately qualified native capabilities only after the following integration and acceptance milestones:

1. define the supported XFS format generation and incompat/ro-compat features;
2. implement bounded host-neutral superblock and allocation-group validation;
3. implement read-only inode, extent, directory and attribute traversal using
   independently manufactured XFS media and malformed fixtures;
4. implement the required B+tree, checksum and ownership validation in the
   canonical core;
5. model log records, replay and recovery before any writer is admitted;
6. add a thin Linux adapter over the canonical core;
7. add allocation/mutation only after transaction, log, CoW/refcount and
   recovery ordering are explicit and destructively qualified;
8. add realtime, quota, reflink, grow and repair features only when their
   persistent contracts are independently implemented and tested;
9. add Windows support only as an adapter over the same canonical engine.

## Safety and failure behaviour

Unknown incompatible features, invalid allocation-group geometry, malformed or
misordered B+trees, contradictory rmap/refcount/allocation state, invalid inode
forks/extents, checksum or owner mismatches, out-of-range block references and
unsafe/ambiguous log-recovery state must fail closed.

A reference Linux implementation being able to mount or repair a volume is not
evidence that Filesystems has independently qualified equivalent write
or recovery support.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable original source, not yet independently qualified for project-native deployment. XFS becomes a Filesystems
first-party implementation only when its canonical engine and applicable
platform adapter have independent implementation, tests and qualification
evidence.

# UFS Filesystem Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

UFS is a family of conventional local block filesystems used by BSD, Sun and
other Unix systems. Filesystems must model explicitly which UFS1/UFS2
and vendor/layout variants it supports rather than treating "UFS" as one
undifferentiated Linux-driver mode.

The target architecture is one canonical host-neutral UFS engine with thin host
adapters.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/ufs/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The copied Linux UFS source includes multiple UFS formats and endian/geometry logic.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/ufs/DESIGN.md` alongside `native/filesystems/ufs/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical ownership

A future `core/` owns the supported UFS format semantics, including:

- variant/superblock discovery and byte-order interpretation;
- cylinder-group geometry and summaries;
- inode representation and direct/indirect block mapping;
- allocation/free-space and inode-allocation rules;
- directory record and namespace semantics;
- fragment/block allocation defined by the selected variant;
- filesystem-defined flags, timestamps and metadata;
- exact mutation/recovery rules for variants where write support is admitted;
- corruption, bounds and cross-structure consistency validation.

Linux VFS objects, buffer/page APIs, credentials, locking and mount/module
lifetime remain Linux-adapter concerns.

## File-boundary rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


The imported `super.c`, `cylinder.c`, `balloc.c`, `ialloc.c`,
`inode.c`, `dir.c`, `namei.c`, `ufs_fs.h` and related files remain
unchanged beneath `linux/`.

Project-owned source must be cut around portable UFS responsibilities and
explicit variant boundaries, not mechanically renamed from Linux.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


1. enumerate exact UFS variants, magic values and byte-order modes supported;
2. implement bounded canonical superblock/geometry detection;
3. implement inode, directory and block-tree traversal for each claimed variant;
4. qualify read-only behaviour with independent UFS1/UFS2/vendor-produced media;
5. add allocation/mutation only per variant with exact safety/recovery contracts;
6. expose the same engine through thin platform adapters.

## Failure policy

Ambiguous/unknown variant state, impossible cylinder geometry, out-of-range
inode/block references, malformed directories and unsupported writable
features must fail closed. Read-only support for one UFS variant must never
silently imply safe mutation of another.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable original source, not yet independently qualified for project-native deployment. UFS becomes a Filesystems
first-party implementation only when its canonical engine and applicable
platform adapters have independent implementation and qualification evidence
for each variant claimed.

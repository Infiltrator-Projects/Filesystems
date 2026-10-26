# System V / Xenix / Coherent Filesystem Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

The SysV filesystem family represented by the Linux `sysv` driver covers
historical System V-derived disk formats, including recognised Xenix, System V
and Coherent variants. These are conventional local block filesystems.

Filesystems therefore applies one canonical host-neutral engine for the
supported SysV-family variants, with thin operating-system adapters.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/sysv/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The copied Linux source includes Xenix, System V and Coherent variant handling.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/sysv/DESIGN.md` alongside `native/filesystems/sysv/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical ownership

A future `core/` owns variant detection plus the filesystem-defined
superblock, inode, free-block/free-inode accounting, directory, block mapping,
byte-order and corruption rules for each explicitly supported variant.

Variant-specific differences must be represented explicitly in the canonical
model rather than hidden in duplicated platform implementations.

Linux buffer/page APIs, VFS objects, credentials, locking and mount/module
lifecycle remain Linux-adapter concerns.

## File-boundary rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


The imported `balloc.c`, `ialloc.c`, `dir.c`, `inode.c`,
`itree.c`, `namei.c`, `super.c` and `sysv.h` remain unchanged in
`linux/`.

Project-owned source must be cut around portable format/variant responsibilities
rather than mechanically preserving those Linux file boundaries.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


1. enumerate and document the exact supported SysV/Xenix/Coherent variants;
2. implement bounded host-neutral variant detection and geometry validation;
3. implement inode, directory and block-tree traversal with independent media;
4. qualify read-only operation and malformed-media rejection per variant;
5. add allocation/mutation only where exact update/recovery semantics are known;
6. add thin platform adapters over the same canonical engine.

## Failure policy

Unknown/ambiguous variants, impossible geometry, out-of-range inode/block
references, corrupt free-list state and malformed directory records must fail
closed. Variant guessing must never turn an uncertain image into a writable
mount.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable original source, not yet independently qualified for project-native deployment. SysV becomes a Filesystems
first-party implementation only when the canonical engine and applicable
adapter have independent implementation and qualification evidence for every
variant claimed.

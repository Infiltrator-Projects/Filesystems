# QNX4 Filesystem Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

QNX4 is a conventional local block filesystem used by QNX 4 systems.
Filesystems therefore applies the standard canonical-engine model:

```text
                    qnx4/core/
             canonical QNX4 semantics
                  /             \
                 /               \
          qnx4/linux/         qnx4/windows/
          thin VFS             thin IFS
          adapter              adapter
```

Filesystem-format rules belong in the host-neutral core. Linux and Windows
objects, cache APIs, mount lifecycle and error translation belong only in their
platform adapters.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/qnx4/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The copied Linux QNX4 code includes bitmap, inode, directory and namespace interpretation.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/qnx4/DESIGN.md` alongside `native/filesystems/qnx4/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical ownership

A future `core/` owns QNX4-defined behaviour including:

- filesystem geometry and superblock validation;
- inode and directory record interpretation;
- allocation bitmap semantics and free-space validation;
- file block/extent mapping defined by the format;
- namespace and filename rules;
- filesystem-defined metadata and timestamps;
- corruption, bounds and consistency validation;
- exact mutation/recovery ordering if writable support is later admitted.

Linux VFS inode/file/dentry objects, page/buffer APIs, kernel credentials,
locking and module/mount lifetime are Linux-adapter concerns rather than QNX4
format semantics.

## File-boundary rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


Upstream files such as `bitmap.c`, `dir.c`, `inode.c`, `namei.c` and
`qnx4.h` remain unchanged below `linux/`.

Project-authored code must be organised around Filesystems
responsibilities and portable filesystem invariants rather than by renaming
those Linux files. Moving or recommenting imported source is never evidence of
an independent rewrite.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


1. document the supported QNX4 format variants and geometry;
2. implement bounded host-neutral superblock/inode/directory/allocation parsing;
3. qualify read-only traversal with independently manufactured QNX4 images and
   malformed-media fixtures;
4. add a thin Linux adapter over the canonical engine;
5. add mutation only with explicit allocation/update/recovery rules and
   destructive qualification;
6. add a Windows adapter against the same canonical engine when appropriate;
7. remove inherited provenance only from implementation bodies that have
   actually been independently replaced.

## Failure policy

Invalid filesystem geometry, out-of-range inode or data references,
contradictory allocation state, malformed directory records and unsupported
format states must fail closed. Read-only compatibility of the current external
Linux provider must not be mistaken for validated first-party write support.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable original source, not yet independently qualified for project-native deployment. QNX4 becomes a Filesystems
first-party implementation only when its canonical engine and applicable
platform adapter have independent implementation and qualification evidence.

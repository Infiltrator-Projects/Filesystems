# QNX6 Filesystem Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

QNX6 Power-Safe is a conventional local block filesystem. Filesystems
therefore applies the standard canonical-engine architecture:

```text
                    qnx6/core/
             canonical QNX6 semantics
                  /             \
                 /               \
          qnx6/linux/         qnx6/windows/
          thin VFS             thin IFS
          adapter              adapter
```

Filesystem-defined structures, validation and recovery semantics belong in the
host-neutral core. Operating-system objects and lifecycle rules belong only in
their adapters.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/qnx6/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The copied QNX6 source includes superblock selection, checksum verification and filesystem object traversal.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/qnx6/DESIGN.md` alongside `native/filesystems/qnx6/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical ownership

A future `core/` owns QNX6-defined behaviour including:

- superblock and filesystem geometry interpretation;
- Power-Safe metadata/snapshot selection and consistency rules;
- inode and directory structures;
- block mapping and allocation metadata;
- checksums and byte-order handling defined by the format;
- namespace and filename semantics;
- corruption and range validation;
- exact recovery/publication rules required by the supported format.

Linux VFS objects, page/buffer-cache APIs, kernel locking, credentials and
mount/module lifetime remain Linux-adapter responsibilities.

## File-boundary rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


The imported `dir.c`, `inode.c`, `namei.c`, `qnx6.h` and
`super_mmi.c` remain unchanged beneath `linux/`.

Project-authored implementation files must be organised around portable QNX6
responsibilities rather than mechanically copying, renaming or recommenting the
Linux reference source.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


1. document the supported QNX6 format/revision and byte-order rules;
2. implement bounded host-neutral superblock, inode, directory and mapping
   decoding;
3. model Power-Safe metadata selection/recovery explicitly;
4. qualify read-only traversal against independent QNX6 media and malformed
   fixtures;
5. implement a thin Linux adapter over the canonical engine;
6. add mutation only with exact transaction/recovery semantics and destructive
   qualification;
7. add a Windows adapter against the same canonical engine when appropriate.

## Failure policy

Invalid superblocks, contradictory Power-Safe metadata state, malformed
directories, out-of-range block references, checksum failures and unsupported
format states must fail closed. A successful read-only mount by the reference
Linux driver is not evidence of safe project-owned mutation.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable original source, not yet independently qualified for project-native deployment. QNX6 becomes a Filesystems
first-party implementation only when the canonical engine and applicable host
adapter have independent implementation and qualification evidence.

# OMFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

OMFS is the Optimized MPEG Filesystem used by certain media-recording devices.
It is a conventional local on-disk block filesystem, so Filesystems
applies the normal canonical-engine architecture.

```text
                  omfs/core/
          canonical OMFS semantics
              /             \
             /               \
       omfs/linux/         omfs/windows/
       thin VFS             thin IFS
       adapter              adapter
```

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/omfs/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The source contains OMFS inode/object and allocation decoding.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/omfs/DESIGN.md` alongside `native/filesystems/omfs/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical-core ownership

The future `core/` owns OMFS-defined behaviour that is independent of host
objects, including as supported:

- volume/superblock and geometry interpretation;
- inode/object record validation;
- allocation bitmap/free-space semantics;
- directory/name lookup and namespace rules;
- extent/file block mapping;
- checksum/integrity rules present in the format;
- mutation ordering and recovery invariants;
- malformed-media and range rejection.

Linux inode/file/dentry objects, buffer/page-cache interfaces, block-device
plumbing, module registration and Linux error/lifetime rules belong in
`linux/`.

Windows IRPs, VCB/FCB/CCB lifetime, Cache Manager integration and NTSTATUS
translation belong in `windows/`.

## File-boundary rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


Upstream files such as `bitmap.c`, `dir.c`, `file.c`, `inode.c`,
`omfs.h` and `omfs_fs.h` retain their original names, licences and
provenance under `linux/`.

Project-authored source must be cut around canonical filesystem responsibilities
and thin host adapters. Moving, renaming or recommenting copied Linux source is
not an independent rewrite.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


1. document the supported OMFS variants, geometry and format limits;
2. implement host-neutral volume, inode/object and allocation decoding;
3. qualify read-only parsing against independently produced media/images and
   malformed fixtures;
4. implement directory and file mapping through the canonical engine;
5. add a thin Linux adapter over that core;
6. add mutation only with explicit allocation/update/recovery qualification;
7. add a Windows adapter against the same canonical implementation when useful.

## Failure policy

Invalid volume geometry, impossible inode/object references, corrupt allocation
state, malformed directory records, out-of-range extents and unsupported
variants must fail closed.

A future writer must not modify media until all affected structures and the
operation's recovery boundary are independently understood and verified.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

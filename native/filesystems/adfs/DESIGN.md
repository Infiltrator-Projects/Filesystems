# ADFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

ADFS (Acorn Disc Filing System) is a conventional local block filesystem used
by RISC OS/Acorn systems. Filesystems therefore applies the normal
canonical-engine architecture:

```text
                  adfs/core/
          canonical ADFS semantics
               /             \
              /               \
       adfs/linux/          adfs/windows/
       Linux VFS             Windows IFS
       adapter               adapter
```

Filesystem rules belong in the canonical engine. Host VFS/IFS objects and
kernel lifecycle belong only in the host adapter.

## Filesystem semantics that belong in the canonical core

The current Linux reference confirms several filesystem-defined areas that must
not remain buried in a Linux adapter when ADFS is rewritten:

- disc-record validation and filesystem geometry;
- sector size and ID-length constraints;
- the allocation/free-space map and its zone/fragment representation;
- indirect disc addresses and object/block mapping;
- classic F and F+ directory formats and their record validation;
- object identity, parent identity, size and directory semantics;
- RISC OS load/execute addresses and 12-bit filetype representation;
- RISC OS owner/public/locked/directory/execute attribute semantics;
- filename limits and filename/filetype presentation rules;
- corruption detection for malformed maps, directory records and geometry;
- read/write ordering and recovery rules for any future mutation support.

These are ADFS semantics. They must be implemented once in `core/` and
consumed by every operating-system adapter.

## Adapter responsibilities

The Linux adapter may own Linux VFS registration, block-device and page/buffer
integration, inode/file/dentry objects, mount option translation, credentials
and Linux error/lifetime rules.

The Windows adapter may own IFS/WDK registration, IRPs, VCB/FCB/CCB lifetime,
Cache Manager integration, native security/attribute presentation and NTSTATUS
translation.

Neither adapter may independently reimplement the allocation map, directory
formats, filetype encoding or other ADFS-defined behaviour.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. Its existing upstream implementation contains F/F+ directory support, allocation-map traversal and inode/file I/O.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/adfs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## Promotion rule

The milestones below describe the remaining format-specific engineering and acceptance tests. They can be implemented by adapting and qualifying the existing source; they do not require a speculative replacement before editing can begin. A separately pursued project-authored rewrite must preserve original provenance until the relevant code has actually been replaced.


1. document the supported ADFS format variants and geometry limits;
2. implement host-neutral disc-record, map, directory and object parsing;
3. qualify parsing against independent ADFS images and malformed fixtures;
4. add read-only mapping/file access through the canonical engine;
5. add a thin Linux adapter without copying filesystem semantics into it;
6. add write support only with explicit transaction/recovery and destructive
   qualification;
7. add a Windows adapter against the same canonical engine when ready;
8. remove inherited provenance only from implementation bodies that have
   actually been independently replaced.

Moving or renaming upstream Linux source is never evidence of authorship.

## Failure policy

Unknown or contradictory disc geometry, invalid map fragments, out-of-range
indirect addresses, malformed directory structures and unsupported variants
must fail closed. A future writer must not mutate media until the complete
affected structure is understood and independently validated.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

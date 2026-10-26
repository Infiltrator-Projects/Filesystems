# BeFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** The original Linux implementation in `linux/` is editable development source with its original licence, copyright and provenance. Earlier frozen-reference and rewrite-before-edit instructions are superseded by [the source ownership policy](../README.md). Direct code modifications are permitted; a qualified native driver and an independently project-authored rewrite are separate achievements.


## Classification

BeFS is the BeOS filesystem and is a conventional local block filesystem.
Filesystems therefore applies the normal canonical-engine architecture:

```text
                  befs/core/
          canonical BeFS semantics
              /             \
             /               \
       befs/linux/         befs/windows/
       thin VFS             thin IFS
       adapter              adapter
```

## Canonical-core ownership

The imported Linux source shows format-specific areas that must ultimately live
in the host-neutral core rather than a Linux wrapper:

- superblock and endian/format interpretation;
- BeFS inode and filesystem type structures;
- B+tree lookup/traversal used by the namespace and metadata;
- datastream/direct/indirect block mapping;
- directory/name interpretation;
- attribute/metadata decoding supported by the format;
- block/address bounds and corruption validation.

Linux VFS inode/file objects, buffer/page interfaces, mount plumbing and kernel
lifetime rules belong only in the Linux adapter.

## Current implementation state

**Editable upstream implementation; not a managed Filesystems native driver.** The actual source is in `linux/` and may be modified directly. The copied source provides the existing Linux BeFS compatibility implementation, primarily read-oriented.

This format is not listed in `data/native-filesystems.tsv`. Neither project-native installation, independently authored provenance, Windows/EFI drivers, nor InfiltratorOS root support follows from the copied source. Its original copyright and licence remain attached to any derivative. Promotion to a supported access path requires reproducible builds, independent interoperability and corruption tests, and runtime qualification of the exact capabilities advertised.

## Source layout and deployment boundary

The checked-in baseline is `native/filesystems/befs/linux/` beside this `DESIGN.md` (not `reference/linux/`). This is actual implementation source for direct modification, not a temporary cache or a guarantee that its unmodified out-of-tree build works. A future `core/` may hold host-neutral filesystem rules extracted from this source, while the existing `linux/` tree becomes or supplies the Linux adapter; a `windows/` adapter is added only when implemented. No invented `kernel/` or second active on-disk parser is necessary. Format-specific host-integration and cross-platform extraction must be validated before claiming those capabilities.

## File-boundary rule

The original files may be edited directly. Changing a filename or translating a body does not by itself change authorship; only actual independent replacement qualifies as project-authored source. Keep working functionality and attribution while moving any genuinely shared format rules to a canonical engine.


Upstream files such as `btree.c`, `datastream.c`, `linuxvfs.c` and
`super.c` remain in the reference tree with their original names.

The permanent project implementation should use responsibility boundaries that
separate host-neutral B+tree/datastream/format semantics from operating-system
integration. It must not preserve `linuxvfs.c` as the architectural centre of
the filesystem merely because that is how the reference driver is organised.

## Promotion sequence

1. document the supported BeFS variants and endian/geometry rules;
2. implement bounded host-neutral superblock/inode/datastream decoding;
3. implement read-only B+tree/directory lookup with independent fixtures;
4. add malformed-media/corruption tests;
5. expose read-only access through a thin Linux adapter;
6. add write support only after exact allocation/update/recovery semantics are
   documented and destructively qualified;
7. add Windows support against the same canonical engine when ready.

## Failure policy

Unsupported variants, invalid B+tree nodes, out-of-range datastream runs,
contradictory endian/geometry fields and malformed directory/metadata records
must fail closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

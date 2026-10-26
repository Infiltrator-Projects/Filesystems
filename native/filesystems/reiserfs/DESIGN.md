# ReiserFS Design and Ownership

> **Working-source and qualification status (26 October 2026).** Actual Linux implementation code lives in `linux/` and is directly editable with original attribution retained. Older frozen-reference or mandatory rewrite-before-edit wording is historical. Direct modification does not establish independent authorship or qualify a filesystem driver. See [the source policy](../README.md).


## Classification

ReiserFS is a conventional local journalled block filesystem. Filesystem
Support therefore applies the standard one-engine architecture:

```text
                 reiserfs/core/
          canonical ReiserFS semantics
               /              \
              /                \
     reiserfs/linux/       reiserfs/windows/
       thin VFS              thin IFS
       adapter               adapter
```

Filesystem-defined tree, item, allocation, journalling and namespace semantics
belong in the host-neutral core. Operating-system objects and lifecycle belong
only in platform adapters.

## Current implementation state

**Editable imported implementation; native deployment not yet qualified.** `native/filesystems/reiserfs/linux/` contains actual Linux filesystem implementation source, which can be developed directly. The copied implementation includes ReiserFS tree, journal and namespace processing.

The original source is not automatically a project-authored implementation. This format is not registered in the 13-member `data/native-filesystems.tsv` deployment manifest. Project-native Linux `.ko`, Windows/EFI support, writing and InfiltratorOS root suitability each require their own applicable integration and independent tests. Existing copyrights and source licences remain intact.

## Current source directory and integration boundary

The actual source layout is `native/filesystems/reiserfs/DESIGN.md` alongside `native/filesystems/reiserfs/linux/`; there is **no `reference/linux/` source tree**. The original `linux/` source is editable implementation code, not a temporary or replaceable research cache. Optional `core/`, `windows/`, `efi/` and other host adapters are created as their real implementation and qualification require, not as a prerequisite for editing existing source. A build must not advertise support it has not passed independent media/runtime tests.

## Canonical ownership

A future `core/` owns ReiserFS-defined behaviour including:

- superblock, format version and feature interpretation;
- keyed balanced-tree and item ordering/traversal semantics;
- formatted/unformatted item representation and tail packing;
- allocation bitmap and object-ID semantics;
- directory item and filename/hash behaviour;
- file mapping and indirect-item rules;
- journal format, transaction ordering, replay and recovery;
- xattr/ACL storage rules defined by the supported format;
- resize rules that alter persistent filesystem geometry;
- corruption, bounds and consistency validation.

Linux buffer/page-cache APIs, VFS inode/file/dentry objects, procfs, kernel
locking, credentials and module/mount lifetime are Linux-adapter concerns.

## File-boundary rule

Editing the copied implementation directly is permitted. Preserve source attribution; renaming or reshaping an imported file does not make it independently project-authored.


Upstream files such as `do_balan.c`, `fix_node.c`, `ibalance.c`,
`lbalance.c`, `stree.c`, `journal.c`, `inode.c` and `super.c`
remain unchanged beneath `linux/`.

Project-authored source must be cut around Filesystems responsibilities
and portable invariants rather than preserving or cosmetically renaming the
Linux translation units. Moving, merging or recommenting imported code is never
evidence that it has crossed the authorship boundary.

## Promotion sequence

The following engineering and qualification milestones apply without banning direct modifications to the existing original source. A project-authored rewrite, if pursued, is a separate provenance milestone and must be evidenced by genuine implementation replacement.


1. document the supported ReiserFS format/revision and feature set;
2. implement bounded host-neutral superblock, key/item and tree decoding;
3. qualify read-only tree, namespace and file traversal with independent images
   and malformed-media fixtures;
4. implement allocation and journal interpretation/recovery in the canonical
   core;
5. add a thin Linux adapter over that same core;
6. add mutation only with explicit transaction/durability/recovery contracts
   and destructive qualification;
7. add Windows support against the same canonical engine when appropriate;
8. retire reference reliance subsystem-by-subsystem only after independent
   replacement and qualification.

## Failure policy

Malformed tree nodes/items, invalid key ordering, out-of-range block references,
contradictory allocation state, unsupported features and unsafe journal/replay
state must fail closed. A reference Linux implementation being able to mount a
volume is not evidence that a future project writer may safely mutate it.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. Promotion into a managed native module requires its own host-integration and runtime qualification.

## Completion rule

The current tree is editable original source, not yet independently qualified for project-native deployment. ReiserFS becomes a Filesystem
Support first-party implementation only when its canonical engine and
applicable platform adapter have independent implementation and qualification
evidence.

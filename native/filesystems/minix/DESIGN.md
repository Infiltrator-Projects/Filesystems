# Minix Filesystem Design and Ownership

> **Working source and managed native Linux status (26 October 2026).** The original implementation in `linux/` remains directly editable and its original provenance applies. This format is already listed in `data/native-filesystems.tsv` as an independently managed Linux module. Native Linux runtime qualification does not imply independent authorship, a full-format implementation audit, Windows/EFI support or an InfiltratorOS root.


## Classification

The Minix filesystem family (v1, v2 and v3) is a conventional local block
filesystem. Filesystems therefore applies the one-canonical-filesystem
architecture: format and filesystem semantics belong in a host-neutral core,
with thin operating-system adapters around that core.

## Current implementation state

**Managed native Linux filesystem — qualified Minix V3 QEMU read/write/remount subset.** The actual original Linux driver, including Minix v1/v2/v3 parsing and VFS operations, resides under `linux/` and is built for the running kernel as independent `minix.ko`. The project native module helper owns build, module selection, signing, rollback and removal. The source retains its original author and licence; using it as a managed driver does not make it independently project-authored.

The qualification record `docs/MINIX_NATIVE_LINUX_QUALIFICATION.md` describes the validated Minix V3 image/runtime path. Variant-wide writing, hostile/corrupt-media recovery, Windows/EFI readers and OS root installation remain separate tests.

## Source layout and adapter boundary

The *actual* editable implementation is `native/filesystems/minix/linux/`, alongside `DESIGN.md`, not a `reference/linux/` tree. The kernel-facing original code is compiled as a managed `minix.ko` independently of other filesystem modules; changes must retain functioning VFS semantics and original attribution. A future host-neutral `core/` can be extracted for other consumers once tested; neither a fabricated `kernel/` directory nor a parallel on-disk implementation is required. The `.so`, Windows driver, firmware and root/Setup outputs are separate acceptance paths.

## Canonical-core ownership

The future core owns Minix-defined host-neutral behaviour including:

- v1/v2/v3 superblock and geometry decoding;
- inode and zone/block addressing;
- inode/zone allocation bitmaps;
- directory record and filename semantics;
- direct and indirect tree mapping;
- timestamps, modes and filesystem-defined metadata;
- free-space/inode accounting;
- corruption and range validation;
- exact mutation and recovery/ordering rules for any supported writer.

Linux VFS objects, buffer/page-cache mechanics, mount/module lifetime and Linux
credential/error translation belong only in the Linux adapter.

## Version rule

Minix v1, v2 and v3 are variants of one filesystem family. Their format
differences must be explicit state in one canonical Minix engine rather than
three unrelated host implementations.

## File-boundary rule

The original translation units `inode.c`, `namei.c`, `bitmap.c` and
`itree_*.c` remain editable under `linux/`; their original provenance must
be retained. They are the current **managed Minix Linux driver**, not a frozen reference.

Project-authored source must be cut around Filesystems responsibilities
and portable Minix invariants. Moving, renaming, merging or recommenting copied
Linux source is never evidence that it has crossed the authorship boundary.

## Promotion sequence

The current Linux native driver is already implemented and has a bounded runtime qualification. The following items describe *additional* portability, completeness and recovery milestones; they are not prerequisites to editing the existing driver and must not erase the recorded Linux qualification.


1. define the supported Minix v1/v2/v3 media variants and limits;
2. implement bounded host-neutral superblock/inode/bitmap decoding;
3. implement read-only directory and file traversal against independent images;
4. qualify malformed geometry, bitmaps, inode references and indirect trees;
5. add a thin Linux adapter over the canonical core;
6. add mutation only after allocation/publication/recovery rules are explicitly
   implemented and destructively qualified;
7. add Windows against the same canonical core when ready.

## Failure policy

Impossible geometry, invalid bitmap references, out-of-range zones/inodes,
malformed directory entries, cyclic/invalid indirect trees and unsupported
format state must fail closed.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. The Linux native module is already managed and has format-bounded runtime qualification; any future capability expansion still requires its own independent evidence.

## Completion rule

The original Linux Minix implementation is an already managed and tested Filesystems Linux provider. Independent project authorship, further media variants, complete crash recovery, portable consumers and root/EFI acceptance remain distinct, unclaimed milestones.

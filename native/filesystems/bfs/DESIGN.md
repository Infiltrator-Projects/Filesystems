# SCO BFS Design and Ownership

> **Working source and managed native Linux status (26 October 2026).** The original implementation in `linux/` remains directly editable and its original provenance applies. This format is already listed in `data/native-filesystems.tsv` as an independently managed Linux module. Native Linux runtime qualification does not imply independent authorship, a full-format implementation audit, Windows/EFI support or an InfiltratorOS root.


## Qualified Linux module state

The actual original Linux BFS implementation now builds as its own `bfs.ko`
using explicit out-of-tree Kbuild registration. Debian 13 Module.symvers-backed
compilation and the independent Linux 6.12 QEMU mount/write/unmount/remount
runtime test passed (Filesystems CI run 39921073843). The Filesystems native
module helper owns installation, source digest, selection, signing, rollback and
removal. This does not qualify a UEFI reader or InfiltratorOS root filesystem.

The original implementation files and provenance are preserved. Older
imported-only and rewrite-first descriptions below are historical and do not
supersede this qualified Linux runtime status.

## Classification

BFS here means the SCO/UnixWare Boot File System. It is a conventional local
block filesystem, so its permanent Filesystems architecture is one
canonical filesystem engine with thin operating-system adapters.

## Current implementation state

**Managed native Linux filesystem — qualified BFS QEMU read/write/remount subset.** The original Linux BFS implementation in `linux/` is built and managed as independent `bfs.ko`; the recorded Debian kernel build and Linux 6.12 QEMU mount/write/remount test establish a bounded Linux runtime milestone (CI run 39921073843). Original upstream author/licence notices remain accurate; native management is not evidence of an independently authored replacement.

Windows/EFI integration, broad corruption/recovery coverage and root installation remain unqualified.

## Source layout and adapter boundary

The *actual* editable implementation is `native/filesystems/bfs/linux/`, alongside `DESIGN.md`, not a `reference/linux/` tree. The kernel-facing original code is compiled as a managed `bfs.ko` independently of other filesystem modules; changes must retain functioning VFS semantics and original attribution. A future host-neutral `core/` can be extracted for other consumers once tested; neither a fabricated `kernel/` directory nor a parallel on-disk implementation is required. The `.so`, Windows driver, firmware and root/Setup outputs are separate acceptance paths.

## Target adapter layout

Retain the managed `linux/` source and extract filesystem-defined semantics to a single `core/` when needed by portable `.so`, Windows or firmware adapters. No such target adapter is implied merely by this document.

## Canonical ownership

The future `core/` owns BFS-defined superblock/inode/directory layout,
allocation and block mapping, filename/directory rules, metadata validation,
free-space/accounting rules and any exact mutation/recovery semantics.

Linux VFS inode/file/dentry objects, buffer/page interfaces, mount lifecycle and
kernel error translation remain in `linux/`. Windows IFS objects and Cache
Manager/IRP integration remain in `windows/`.

## Promotion rule

The current Linux native driver is already implemented and has a bounded runtime qualification. The following items describe *additional* portability, completeness and recovery milestones; they are not prerequisites to editing the existing driver and must not erase the recorded Linux qualification.


BFS is **already deployed as an independently managed native Linux driver**, with a bounded QEMU runtime qualification. Additional portable-core extraction, malformed-media coverage, Windows/EFI integration and recovery qualification remain outstanding.
Write support requires a separately reviewed durability/recovery contract.

Moving or renaming upstream source does not cross the authorship boundary.

## Copied implementation provenance

The editable `linux/` source contains original implementation code copied from the pinned Linux v6.12.107 source baseline. Original copyright, licence and attribution remain part of that code. Source refresh is not automatic and must never replace Filesystems modifications. The Linux native module is already managed and has format-bounded runtime qualification; any future capability expansion still requires its own independent evidence.

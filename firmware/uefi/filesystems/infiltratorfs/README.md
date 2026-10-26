# InfiltratorFS UEFI integration

Boot may load an InfiltratorFS boot-payload volume only through a reader
qualified for the actual current InfiltratorFS on-disk format. The canonical
format engine and maintained EFI implementation are owned by the separate
`Infiltrator-Projects/InfiltratorFS` repository. This folder is a **legacy,
pinned Format 0.18 migration snapshot**, not a current or production
InfiltratorFS EFI driver. It must not be selected merely because the source
exists here.

The historical directory was intended as a UEFI integration layer. It is deliberately based on the canonical portable InfiltratorFS format/core implementation rather than the Linux kernel VFS adapter; Linux kernel code cannot be used directly in UEFI.

## Source provenance

The initial UEFI port is pinned to Infiltrator-Projects/InfiltratorFS commit:

`0451f95cadf57484640d87eea2037e067bc7ed6f`

At that revision the project reports InfiltratorFS 0.18.94 / on-disk Format 0.18. The vendored source retains its original GPL-3.0-or-later notices and a copy of the InfiltratorFS GPLv3 licence.

## Architecture

The target architecture is read-only at boot time:

1. EFI Disk I/O is exposed to the portable InfiltratorFS reader through an `infs_storage` backend.
2. The canonical InfiltratorFS checkpoint, object, namespace, sparse-file, inline-data and compression logic remains authoritative.
3. rEFInd's FSW layer is used only as the EFI Simple File System bridge.
4. File data returned by InfiltratorFS is exposed through FSW buffer extents, so compressed or non-contiguous InfiltratorFS files do not need to be misrepresented as simple physical extents.
5. No filesystem writes are enabled in the boot manager.

The Linux `kernel/` implementation is not copied into the boot manager. The UEFI port follows the portable `include/infilfs` and `src` implementation so that there is one on-disk-format truth.

## Status

The historical portable source is present only as an unshipped migration
snapshot; it is not the canonical current reader. Filesystems' maintained
EFI build excludes this Format 0.18 adapter. The duplicate implementation
must not be promoted or used as a substitute for the current InfiltratorFS
repository's independently qualified reader. Any new firmware integration
must verify the current format generation, directories, symlinks, ordinary,
sparse, inline and compressed reads, and real boot-media I/O before release.
The ongoing removal of the obsolete fork is tracked separately in
Filesystems PR #39.

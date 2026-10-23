# Filesystem source availability

41 catalogue formats exist. At this source-recovery revision:

- 9 formats have existing native Linux driver source and managed deployment:
  EXT2, EXT3, EXT4, OFS, FFS, SFS, SFS2, PFS3 and Minix.
- 26 additional formats now have the **actual copied Linux source**, editable
  under `native/filesystems/<id>/linux/`. This includes the 38-file, hash-verified original APFS out-of-tree module and NTFS source recovered
  from the earlier NTFS3 copy in this repository's Git history.
- 6 remain without a complete dedicated in-repository original-source copy:
  CP/M, Fosfat, InfiltratorFS, VMFS3/VMFS5, VMFS6 and ZFS.

This is an inventory of available implementation source, not a claim that all
41 have project-qualified native Linux modules, Windows drivers or firmware
readers. InfiltratorFS source is independently maintained in its own project,
and its existing EFI vendor subset is not a complete source copy here.

The six missing source baselines must not be represented as completed by
design documents, package names, third-party runtime availability or empty
directories. The source recovery also does not change previous release
binaries or qualify any previously unqualified drivers.

The previous SFS, SFS2 and PFS3 sources are also retained byte-for-byte under their respective `original/` directories, separately from the qualified native drivers. No obsolete `reference/` directories remain under `native/filesystems/`.

Minix was promoted from imported source to the managed Linux module set after an original-source build, symbol-resolved Debian 13 build, and a successful Linux 6.12 QEMU read/write/remount runtime qualification. The independently editable original Linux source and its attribution are retained. This does not qualify Minix as an InfiltratorOS root or establish a matching EFI driver.

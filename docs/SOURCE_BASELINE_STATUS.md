# Filesystem source availability

41 catalogue formats exist. At this source-recovery revision:

- 8 formats have existing native Linux driver source and managed deployment:
  EXT2, EXT3, EXT4, OFS, FFS, SFS, SFS2 and PFS3.
- 26 additional formats now have the **actual copied Linux source**, editable
  under `native/filesystems/<id>/linux/`. This includes NTFS source recovered
  from the earlier NTFS3 copy in this repository's Git history.
- 7 remain without a complete dedicated in-repository original-source copy:
  APFS, CP/M, Fosfat, InfiltratorFS, VMFS3/VMFS5, VMFS6 and ZFS.

This is an inventory of available implementation source, not a claim that all
41 have project-qualified native Linux modules, Windows drivers or firmware
readers. InfiltratorFS source is independently maintained in its own project,
and its existing EFI vendor subset is not a complete source copy here.

The seven missing source baselines must not be represented as completed by
design documents, package names, third-party runtime availability or empty
directories. The source recovery also does not change version 0.5.55 release
binaries or qualify any previously unqualified drivers.

The previous SFS, SFS2 and PFS3 sources are also retained byte-for-byte under their respective `original/` directories, separately from the qualified native drivers. No obsolete `reference/` directories remain under `native/filesystems/`.

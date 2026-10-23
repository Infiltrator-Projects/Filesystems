# Filesystem source availability

The current format catalogue contains **41 identities**. As of version 0.5.68:

- **10** project-managed native Linux drivers: EXT2, EXT3, EXT4, OFS, FFS,
  SFS, SFS2, PFS3, Minix and BFS (`data/native-filesystems.tsv`).
- **25** other actual copied Linux driver implementations that are editable
  under `native/filesystems/<id>/linux/`
  (`data/imported-source-filesystems.txt`).
- **1** original editable userspace implementation for Fosfat/Smaky under
  `native/filesystems/fosfat/userspace/`
  (`data/imported-userspace-filesystems.txt`). This is not a `.ko`.
- **5** without a complete source baseline in this repo: CP/M, InfiltratorFS,
  VMFS3/VMFS5, VMFS6 and ZFS. InfiltratorFS maintains its own native driver
  in a separate repository.

Source presence is not equivalent to build success, a qualified installable
kernel module, EFI boot-reader support, or safe root-filesystem semantics.

The original SFS, SFS2 and PFS3 implementations are also preserved in
their per-format `original/` directories. There are no obsolete
`reference/` source quarantines under `native/filesystems/`.

Minix and BFS passed Linux 6.12 QEMU mount/write/remount qualification.
Dedicated original-driver runtime workflows have been added for exFAT,
NTFS3 and UDF but these formats stay outside the managed manifest until
their real runtime and data integrity checks pass.

Fosfat's actual original source is now present, but needs a native Linux
VFS adapter; no read/write kernel driver is being claimed.

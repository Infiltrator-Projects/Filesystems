# Filesystem source availability

**41 catalogue formats**, of which **37** have editable implementation
source in this repository:

- **12** project-managed native Linux drivers: EXT2, EXT3, EXT4, OFS, FFS,
  SFS, SFS2, PFS3, Minix, BFS, NTFS3 (single NTFS identity) and exFAT.
  These are independently deployed according to `data/native-filesystems.tsv`.
- **23** additional copied Linux filesystem implementations, directly editable
  under their `native/filesystems/<id>/linux/` paths.
  They are not yet part of the managed installer.
- **2** actual userspace-origin implementations for CP/M and Smaky/Fosfat,
  editable under `native/filesystems/<id>/userspace/`. Neither has a native
  Linux VFS module yet.
- **4** complete code baselines not yet copied into this repository:
  InfiltratorFS (original separately maintained driver), VMFS3/VMFS5,
  VMFS6 and ZFS.

Source availability, independent native runtime qualification, portable
EFI reader qualification and InfiltratorOS root installation are separate.

Minix and BFS passed Linux 6.12 QEMU write/remount; NTFS3 passed write/remount
plus independent ntfscat; exFAT passed write/remount plus independent
fsck.exfat. UDF still has an independent QEMU qualification workflow pending.
These tests do not certify the entire format feature set or physical media.

Original SFS/SFS2/PFS3 source snapshots remain in per-format `original/`
directories. No old reference-only quarantines remain. Source copyrights
and licensing are retained in the actual copied code.

# Filesystem source availability

**41 catalogue formats**, of which **37** have editable source
implementations in this repository:

- **11** managed Linux filesystem module identities: EXT2, EXT3, EXT4, OFS,
  FFS, SFS, SFS2, PFS3, Minix, BFS and NTFS (module `ntfs3`).
  Authority: `data/native-filesystems.tsv`.
- **24** other original Linux implementation trees, editable under
  `native/filesystems/<id>/linux/`, but not yet natively managed.
  Authority: `data/imported-source-filesystems.txt`.
- **2** actual copied userspace filesystem implementations under
  `native/filesystems/<id>/userspace/`: CP/M and Smaky/Fosfat.
  Both still require native Linux VFS adapters; no `.ko` is claimed.
  Authority: `data/imported-userspace-filesystems.txt`.
- **4** source imports remain: InfiltratorFS (separate original project),
  VMFS3/VMFS5, VMFS6 and ZFS.

Native registration is not a claim of comprehensive read/write safety,
EFI reader readiness, physical-media qualification or bootable root.

Minix and BFS passed Linux 6.12 QEMU mount/write/remount qualification.
NTFS3 passed QEMU native mount/write/remount and independent ntfscat
readback in workflow run 39931921927.
Dedicated exFAT and UDF original-source native QEMU workflows have also
been added; both remain unpromoted until their results pass.

SFS/SFS2/PFS3 retained original sources remain in their separate
`original/` directories. No obsolete reference-only source quarantine
remains in the format source tree. Original copyright notices are retained.

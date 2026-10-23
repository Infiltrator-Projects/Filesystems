# Filesystem source availability

**41 catalogue formats**, of which **37** now have editable source
implementations in this repository:

- **10** managed native Linux driver identities: EXT2, EXT3, EXT4, OFS,
  FFS, SFS, SFS2, PFS3, Minix and BFS
  (`data/native-filesystems.tsv`).
- **25** additional actual copied Linux filesystem implementations under
  `native/filesystems/<id>/linux/`
  (`data/imported-source-filesystems.txt`).
- **2** actual copied userspace filesystem implementations under
  `native/filesystems/<id>/userspace/`: Smaky/Fosfat (original Fosfat)
  and CP/M (original cpmtools). Neither has a Linux `.ko` yet.
  (`data/imported-userspace-filesystems.txt`).

**4 formats still lack a complete source copy in this repository:**
InfiltratorFS (maintained as its own independent driver repository),
VMFS3/VMFS5, VMFS6 and ZFS.

This is a source inventory, not a claim that original modules have passed
host integration, corrupt-media safety, independent writes/remounts, EFI
reader qualification or root-filesystem installation.

Minix and BFS have independent Linux 6.12 QEMU native mount/write/remount
evidence. exFAT, NTFS3 and UDF have dedicated native runtime workflows
but cannot be promoted until those tests and independent data validation pass.

Original SFS, SFS2 and PFS3 implementation snapshots are also retained
in per-filesystem `original/` directories. There are no obsolete source
`reference/` quarantine directories under `native/filesystems/`.

CP/M and Fosfat source has been copied with its original copyright and
licence material, not implemented anew or claimed as kernel drivers.

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Do not reintroduce already extracted filesystem sources into Filesystems."""
from pathlib import Path
from sys import argv
root=Path(argv[1] if len(argv)>1 else ".").resolve()
owned=("apfs","btrfs","exfat","ext2","ext3","ext4","fat","ffs",
       "hfs","hfsplus","minix","ntfs","ofs","pfs3","sfs","sfs2",
       "ufs","xfs","zfs")
for name in owned:
    assert not (root/"native/filesystems"/name).exists(), f"Extracted source returned: {name}"
assert not (root/"native/providers").exists(), "Provider submodules must not return"
gitmodules=(root/".gitmodules").read_text()
assert "src/infiltratr-common" in gitmodules
assert "native/providers/" not in gitmodules
cmake=(root/"CMakeLists.txt").read_text()
for suffix in ("native/filesystems/fat/","native/filesystems/ext2/",
               "native/filesystems/ofs/","native/filesystems/minix/"):
    assert suffix not in cmake, f"Retired build source returned: {suffix}"
assert not (root/".github/workflows/apt-publication.yml").exists()
print("FILESYSTEMS-RETIREMENT-BOUNDARY-PASS")

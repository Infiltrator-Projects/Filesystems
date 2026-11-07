#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Transitional source ownership guard. Extracted formats are forbidden here.
set -euo pipefail
cd "$(dirname "$0")/.."
manifest_values() { grep -Ev '^[[:space:]]*(#|$)' "$1"; }
source_manifest=data/source-filesystems.txt
native_manifest=data/native-filesystems.tsv
imported_manifest=data/imported-source-filesystems.txt
userspace_manifest=data/imported-userspace-filesystems.txt
for file in "$source_manifest" "$native_manifest" "$imported_manifest" "$userspace_manifest"; do
    test -s "$file"
done
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
manifest_values "$source_manifest" | sort > "$work/expected"
find native/filesystems -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort > "$work/actual"
diff -u "$work/expected" "$work/actual"
test "$(sort -u "$work/expected" | wc -l)" -eq "$(wc -l < "$work/expected")"
# Those standalone owners must never be vendored, mirrored or submoduled here.
test ! -e native/providers
for fs in apfs btrfs exfat ext2 ext3 ext4 fat ffs hfs hfsplus minix ntfs ofs pfs3 sfs sfs2 ufs xfs zfs; do
    test ! -e "native/filesystems/$fs" || {
        echo "Extracted filesystem unexpectedly returned: $fs" >&2
        exit 1
    }
done
while IFS= read -r fs; do
    test -s "native/filesystems/$fs/DESIGN.md"
done < "$work/expected"
while IFS=$'\t' read -r fs module; do
    [[ -n "$fs" && "$fs" != \#* ]] || continue
    grep -Fxq "$fs" "$work/expected"
    test -f "native/filesystems/$fs/linux/Makefile"
    find "native/filesystems/$fs/linux" -name '*.c' -type f -print -quit | grep -q .
done < "$native_manifest"
while IFS= read -r fs; do
    grep -Fxq "$fs" "$work/expected"
    test -d "native/filesystems/$fs/linux"
done < <(manifest_values "$imported_manifest")
while IFS= read -r fs; do
    grep -Fxq "$fs" "$work/expected"
    test -s "native/filesystems/$fs/userspace/COPYING"
done < <(manifest_values "$userspace_manifest")
grep -Fq 'src/infiltratr-common' .gitmodules
! grep -Fq 'native/providers/' .gitmodules
echo "CENTRAL-FILESYSTEM-EXTRACTION-BOUNDARY-PASS"

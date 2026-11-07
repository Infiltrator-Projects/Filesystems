#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Repository/source boundary contract.
#
# Keep mutable filesystem-set knowledge in data manifests, not in CI YAML.
# This script enforces invariants that are genuinely about implementation
# boundaries and can therefore be run locally or by any CI system.
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT HUP INT TERM

source_manifest=data/source-filesystems.txt
retired_manifest=data/retired-source-identities.txt
native_manifest=data/native-filesystems.tsv
desktop_manifest=data/desktop-filesystems.tsv

for manifest in "$source_manifest" "$retired_manifest" "$native_manifest" "$desktop_manifest"; do
    [[ -s "$manifest" ]] || {
        echo "Required source contract manifest is missing or empty: $manifest" >&2
        exit 1
    }
done

manifest_values() {
    grep -Ev '^[[:space:]]*(#|$)' "$1"
}

test ! -e tools/shape-ext2.sh
test ! -e tools/shape-ext3.sh
test ! -e tools/shape-ext4.sh
test ! -e tools/harden-ext-ranges.py

# Source trees are declared once in data/source-filesystems.txt. CI does not
# maintain its own copy of the catalogue subset.
manifest_values "$source_manifest" | sort > "$work/source.expected"
if [[ $(wc -l < "$work/source.expected") -ne \
      $(sort -u "$work/source.expected" | wc -l) ]]; then
    echo "Duplicate filesystem in $source_manifest" >&2
    exit 1
fi
# Local source trees and immutable Git submodules are both part of the
# authoritative source inventory. Extracted formats must not be copied back.
resolve_source_dir() {
    local fs="$1"
    local local_dir="native/filesystems/$fs"
    if [[ -d "$local_dir" ]]; then
        printf '%s\n' "$local_dir"
        return
    fi
    local owner
    case "$fs" in
        apfs) owner=APFS ;;
        btrfs) owner=Btrfs ;;
        exfat) owner=exFAT ;;
        ext2) owner=EXT2 ;;
        ext3) owner=EXT3 ;;
        ext4) owner=EXT4 ;;
        ffs) owner=FFS ;;
        hfs) owner=HFS ;;
        hfsplus) owner=HFSPlus ;;
        minix) owner=Minix ;;
        ntfs) owner=NTFS ;;
        ofs) owner=OFS ;;
        pfs3) owner=PFS3 ;;
        sfs) owner=SFS ;;
        sfs2) owner=SFS2 ;;
        ufs) owner=UFS1 ;;
        xfs) owner=XFS ;;
        zfs) owner=ZFS ;;
        *) echo "No independent source owner for $fs" >&2; return 1 ;;
    esac
    local provider="native/providers/Filesystem-$owner"
    local source="$provider/native/filesystems/$fs"
    [[ -f "$provider/.git" && -d "$source" ]] || {
        echo "Missing pinned independent source: $source" >&2
        return 1
    }
    printf '%s\n' "$source"
}
ofs_source=$(resolve_source_dir ofs)
ffs_source=$(resolve_source_dir ffs)
ext2_source=$(resolve_source_dir ext2)
{
    find native/filesystems -mindepth 1 -maxdepth 1 -type d -printf '%f\n'
    while IFS= read -r fs; do
        if [[ ! -d "native/filesystems/$fs" ]]; then
            resolve_source_dir "$fs" >/dev/null
            printf '%s\n' "$fs"
        fi
    done < "$work/source.expected"
} | sort -u > "$work/source.actual"
diff -u "$work/source.expected" "$work/source.actual"

while IFS= read -r fs; do
    [[ "$fs" =~ ^[A-Za-z0-9_.+-]+$ ]] || {
        echo "Invalid source filesystem token: $fs" >&2
        exit 1
    }
    test -s "$(resolve_source_dir "$fs")/DESIGN.md" || {
        echo "Missing design/provenance contract for $fs" >&2
        exit 1
    }
done < "$work/source.expected"

# Retired provider/transport/overlay/archive identities are kept in one data
# file so adding or removing an historical guard does not require editing CI.
while IFS= read -r fs; do
    [[ "$fs" =~ ^[A-Za-z0-9_.+-]+$ ]] || {
        echo "Invalid retired filesystem token: $fs" >&2
        exit 1
    }
    test ! -e "$(resolve_source_dir "$fs")" || {
        echo "Retired non-format source tree returned: $fs" >&2
        exit 1
    }
done < <(manifest_values "$retired_manifest")

# Native deployment owns only filesystem/module identity. Desktop membership
# and variants have a separate manifest and are forbidden from leaking back
# into this contract.
awk -F '\t' '
    BEGIN { ok = 1 }
    /^[[:space:]]*#/ || /^[[:space:]]*$/ { next }
    NF != 2 {
        print "Invalid native manifest row: " $0 > "/dev/stderr"
        ok = 0
        next
    }
    $1 !~ /^[A-Za-z0-9_.+-]+$/ || $2 !~ /^[A-Za-z0-9_.+-]+$/ {
        print "Invalid native filesystem/module token: " $0 > "/dev/stderr"
        ok = 0
    }
    seen_fs[$1]++ {
        print "Duplicate native filesystem: " $1 > "/dev/stderr"
        ok = 0
    }
    seen_module[$2]++ {
        print "Duplicate native module: " $2 > "/dev/stderr"
        ok = 0
    }
    END { exit ok ? 0 : 1 }
' "$native_manifest"

while IFS=$'\t' read -r fs module; do
    [[ -n "$fs" && "${fs:0:1}" != "#" ]] || continue
    grep -Fxq "$fs" "$work/source.expected" || {
        echo "Native filesystem $fs is not present in $source_manifest" >&2
        exit 1
    }
    test -f "$(resolve_source_dir "$fs")/linux/Makefile"
    find "$(resolve_source_dir "$fs")/linux" -type f -name '*.c' -print -quit |
        grep -q . || {
        echo "Managed native filesystem has no source code: $fs" >&2
        exit 1
    }
    # The original Linux VFS implementation is sufficient when it is
    # self-contained: a manufactured empty canonical core is not required.
    if [[ ! -d "$(resolve_source_dir "$fs")/core" ]]; then
        grep -Eq "^[[:space:]]*obj-m[[:space:]]*[+:?]?=[[:space:]]*${module}[.]o" \
            "$(resolve_source_dir "$fs")/linux/Makefile" || {
            echo "Core-less native driver is not explicitly standalone: $fs" >&2
            exit 1
        }
    fi
done < "$native_manifest"

# Desktop policy may reference only a managed native filesystem. Its variant
# namespace is unique and positive, but it has no authority over native module
# build/deployment membership.
awk -F '\t' -v native="$native_manifest" '
    BEGIN {
        ok = 1
        while ((getline line < native) > 0) {
            if (line ~ /^[[:space:]]*#/ || line ~ /^[[:space:]]*$/) continue
            split(line, fields, "\t")
            managed[fields[1]] = 1
        }
        close(native)
    }
    /^[[:space:]]*#/ || /^[[:space:]]*$/ { next }
    NF != 2 {
        print "Invalid desktop manifest row: " $0 > "/dev/stderr"
        ok = 0
        next
    }
    !managed[$1] {
        print "Desktop filesystem is not managed natively: " $1 > "/dev/stderr"
        ok = 0
    }
    $2 !~ /^[1-9][0-9]*$/ {
        print "Invalid desktop variant: " $0 > "/dev/stderr"
        ok = 0
    }
    seen_fs[$1]++ {
        print "Duplicate desktop filesystem: " $1 > "/dev/stderr"
        ok = 0
    }
    seen_variant[$2]++ {
        print "Duplicate desktop variant: " $2 > "/dev/stderr"
        ok = 0
    }
    END { exit ok ? 0 : 1 }
' "$desktop_manifest"

# OFS and FFS must not share AFFS-family runtime state. Only the tiny generic
# kernel compatibility layer may be shared.
test -s native/platform/linux_kernel/vfs_compat.h
test ! -e native/platform/linux_kernel/amiga_affs_compat.h
test -s ${ofs_source}/linux/affs_compat.h
test -s ${ffs_source}/linux/affs_compat.h
grep -Fq '#include "affs_compat.h"' ${ofs_source}/linux/linux_adapter.h
grep -Fq '#include "affs_compat.h"' ${ffs_source}/linux/linux_adapter.h
! grep -R -Fq '../../../platform/linux_kernel/amiga_affs_compat.h' \
    ${ofs_source} ${ffs_source}

test ! -e native/filesystems/amiga_common
test ! -e native/primitives/amiga_dos
rewritten=(
    ${ofs_source}/linux/core_bridge.c
    ${ofs_source}/linux/storage.c
    ${ofs_source}/linux/namespace.c
    ${ofs_source}/linux/lifecycle.c
    ${ofs_source}/linux/linux_adapter.h
    ${ofs_source}/core/ofs_disk_layout.h
    ${ofs_source}/core/ofs_primitives.c
    ${ffs_source}/linux/core_bridge.c
    ${ffs_source}/linux/storage.c
    ${ffs_source}/linux/namespace.c
    ${ffs_source}/linux/lifecycle.c
    ${ffs_source}/linux/linux_adapter.h
    ${ffs_source}/core/ffs_disk_layout.h
    ${ffs_source}/core/ffs_primitives.c
)
for file in "${rewritten[@]}"; do
    test -s "$file"
    ! grep -Eiq 'linux/fs/affs|Hans-Joachim Widmaier|Ray Burr|Eric Youngdale|Linus Torvalds' "$file"
done
! grep -R -Eq 'IFS_FFS_IS_OFS|SF_OFS|AFFS_MOUNT_SF_OFS|affs_aops_ofs|ifs_ffs_ofs_|AFFS_DATA_HEAD|AFFS_DATA\(' \
    ${ffs_source}/linux

# Copied Linux code is editable working source, not a disposable reference cache.
# Only the native deployment manifest may turn code into an installable module.
imported_manifest=data/imported-source-filesystems.txt
test -s "$imported_manifest"
manifest_values "$imported_manifest" | sort > "$work/imported.expected"
test "$(wc -l < "$work/imported.expected")" -eq \
     "$(sort -u "$work/imported.expected" | wc -l)"
while IFS= read -r fs; do
    grep -Fxq "$fs" "$work/source.expected" || {
        echo "Undeclared imported filesystem source: $fs" >&2
        exit 1
    }
    if awk -F '\t' -v fs="$fs" '$1 == fs { found = 1 } END { exit !found }' "$native_manifest"; then
        echo "Imported-only source was added to the native deployment manifest: $fs" >&2
        exit 1
    fi
    test -d "$(resolve_source_dir "$fs")/linux" || {
        echo "Missing actual imported implementation: $fs" >&2
        exit 1
    }
    find "$(resolve_source_dir "$fs")/linux" -type f \( -name '*.c' -o -name '*.h' \) -print -quit |
      grep -q . || {
        echo "No implementation source present: $fs" >&2
        exit 1
    }
done < "$work/imported.expected"

# Original userspace implementation code is also editable and is never a
# kernel module until a real Linux VFS adapter receives qualification.
userspace_manifest=data/imported-userspace-filesystems.txt
test -s "$userspace_manifest"
manifest_values "$userspace_manifest" | sort > "$work/userspace.expected"
test "$(wc -l < "$work/userspace.expected")" -eq \
     "$(sort -u "$work/userspace.expected" | wc -l)"
while IFS= read -r fs; do
    grep -Fxq "$fs" "$work/source.expected" || {
        echo "Unknown original userspace filesystem: $fs" >&2
        exit 1
    }
    if grep -Fxq "$fs" "$work/imported.expected" ||
       awk -F '\t' -v fs="$fs" '$1 == fs { found = 1 } END { exit !found }' "$native_manifest"; then
        echo "Userspace source cannot masquerade as a native module: $fs" >&2
        exit 1
    fi
    source_dir="$(resolve_source_dir "$fs")/userspace"
    test -s "$source_dir/COPYING" || {
        echo "Original source licence/provenance missing: $fs" >&2
        exit 1
    }
    find "$source_dir" -type f -name '*.c' -print -quit | grep -q . || {
        echo "Actual original userspace filesystem implementation missing: $fs" >&2
        exit 1
    }
done < "$work/userspace.expected"

if find native/filesystems -type d -name reference -print -quit | grep -q .; then
    echo "Obsolete source-reference directory has returned" >&2
    exit 1
fi
! grep -Fq '/reference' CMakeLists.txt
while IFS=$'\t' read -r fs desktop_variant; do
    [[ -n "$fs" && "${fs:0:1}" != "#" ]] || continue
    ! grep -Fq 'reference' "$(resolve_source_dir "$fs")/linux/Makefile"
done < "$desktop_manifest"

# Windows EXT2 must consume the canonical EXT2 engine rather than a second
# filesystem implementation.
for part in adapter_support name_translation file_dispatch directory_dispatch volume_lifecycle driver_entry; do
    test -s "${ext2_source}/windows/$part.inc"
    grep -Fq "#include \"$part.inc\"" ${ext2_source}/windows/ext2_driver.c
done
test "$(wc -l < ${ext2_source}/windows/ext2_driver.c)" -lt 80
test ! -e ${ext2_source}/windows/ext2_compat.h
! grep -R -E 'archive/extfs-for-windows|core/extfs(_classic_resize|_resize_dispatch)?\.c' \
    ${ext2_source}/windows windows/build CMakeLists.txt
grep -Fq '..\core\ext2_core.c' \
    ${ext2_source}/windows/filesystems_ext2.vcxproj
grep -Fq '..\core\ext2_engine.c' \
    ${ext2_source}/windows/filesystems_ext2.vcxproj
grep -Fq '#include "../core/ext2_engine.h"' \
    ${ext2_source}/windows/ext2_driver.h
grep -Fq '#include "../core/ext2_engine.c"' \
    ${ext2_source}/linux/core_bridge.c
grep -Fq 'metadata.o' ${ext2_source}/linux/Makefile
grep -Fq -- '-DCONFIG_EXT2_FS_XATTR=1' ${ext2_source}/linux/Makefile
grep -Fq -- '-DCONFIG_EXT2_FS_POSIX_ACL=1' ${ext2_source}/linux/Makefile
grep -Fq -- '-DCONFIG_EXT2_FS_SECURITY=1' ${ext2_source}/linux/Makefile

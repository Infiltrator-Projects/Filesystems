#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# exFAT.so local module compiler. Never install, load or replace exfat.ko.
set -euo pipefail
export LC_ALL=C
self_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
app_dir=$(cd -- "$self_dir/.." && pwd -P)
root="$app_dir/Sources/exfat/native/filesystems"
src_ex="$root/exfat/module"
src_port="$root/fat/portable"
destination=/System/Filesystems
module="$destination/exFAT.so"
fatal(){ echo "exFAT: $*" >&2; exit 1; }
[[ $# -eq 1 ]] || fatal "usage: exfat-module-helper install|remove|status"
safe_directory(){
    local dir="$1" mode
    [[ -d "$dir" && ! -L "$dir" ]] || fatal "unsafe directory: $dir"
    [[ "$(stat -c %u -- "$dir")" == 0 ]] || fatal "source directory not root-owned: $dir"
    mode=$(stat -c %a -- "$dir")
    (( (8#$mode & 8#022) == 0 )) || fatal "source directory writable by other users: $dir"
}
protected_module(){
    if [[ -x /usr/bin/dpkg-query ]] &&
       /usr/bin/dpkg-query -S -- "$module" >/dev/null 2>&1; then
        fatal "exFAT.so belongs to a Debian package; refusing mutation"
    fi
}
verify_module(){
    local file="$1"
    [[ -f "$file" && ! -L "$file" ]] || fatal "not a regular module"
    /usr/bin/readelf -h "$file" | grep -Eq 'Type:.*DYN' ||
        fatal "not an ELF shared object"
    for v in 3 4 5; do
        /usr/bin/readelf -Ws "$file" | grep -F "infiltrator_filesystem_module_v$v" >/dev/null ||
            fatal "missing exFAT V$v ABI"
        /usr/bin/readelf -p .ifsmeta "$file" |
            grep -F "IFS-MODULE:exfat:$v" >/dev/null ||
            fatal "incorrect exFAT V$v format identity"
    done
    ! /usr/bin/readelf -d "$file" | grep -Eq '[(]NEEDED[)]' ||
        fatal "unexpected runtime library dependency"
}
safe_source_tree(){
    local dir name mode
    for dir in "$(dirname -- "$app_dir")" "$app_dir" "$self_dir" \
        "$app_dir/Sources" "$app_dir/Sources/exfat" \
        "$app_dir/Sources/exfat/native" "$root" \
        "$root/exfat" "$src_ex" "$root/fat" "$src_port"; do
        safe_directory "$dir"
    done
    for name in "$src_ex/exfat_module.c" "$src_ex/exfat_api.h" \
        "$src_port/fatfs_module.c" "$src_port/fatfs_module.h" \
        "$src_port/ff.c" "$src_port/ff.h" "$src_port/ffconf.h" \
        "$src_port/ffunicode.c" "$src_port/fatfs_libc.c" \
        "$src_port/diskio.h"; do
        [[ -f "$name" && ! -L "$name" ]] || fatal "missing or symlinked source: $name"
        [[ "$(stat -c %u -- "$name")" == 0 ]] || fatal "not root-owned: $name"
        mode=$(stat -c %a -- "$name")
        (( (8#$mode & 8#022) == 0 )) || fatal "writable by others: $name"
    done
}
case "$1" in
status)
    [[ -d /System && -d "$destination" && ! -L /System && ! -L "$destination" ]] || exit 3
    safe_directory /System
    safe_directory "$destination"
    [[ -f "$module" && ! -L "$module" ]] || exit 3
    [[ "$(stat -c %u -- "$module")" == 0 ]] || fatal "module is not root-owned"
    verify_module "$module"
    printf 'installed\n'
    ;;
install)
    [[ "$(id -u)" == 0 ]] || fatal "requires authorisation"
    protected_module
    safe_source_tree
    [[ -x /usr/bin/cc && -x /usr/bin/readelf ]] || fatal "C toolchain unavailable"
    [[ ! -L /System && ! -L "$destination" && ! -L "$module" ]] ||
        fatal "symlinked destination refused"
    [[ -d /System ]] || mkdir -m 0755 /System
    [[ -d "$destination" ]] || mkdir -m 0755 "$destination"
    safe_directory /System
    safe_directory "$destination"
    temporary=$(mktemp "$destination/.exFAT.so.XXXXXXXX")
    trap 'rm -f -- "$temporary"' EXIT
    /usr/bin/cc -std=c11 -O2 -fPIC -fno-builtin -fno-stack-protector \
        -ffreestanding -fvisibility=hidden -nostdlib -shared \
        -Wl,-z,defs -Wl,-z,relro -Wl,-z,now -DIFS_FATFS_ONLY=64 \
        -I "$src_ex" -I "$src_port" \
        "$src_ex/exfat_module.c" "$src_port/ff.c" \
        "$src_port/ffunicode.c" "$src_port/fatfs_libc.c" \
        -o "$temporary" || fatal "compilation failed, installed module unchanged"
    verify_module "$temporary"
    chmod 0644 "$temporary"
    chown root:root "$temporary"
    mv -fT -- "$temporary" "$module"
    trap - EXIT
    echo "Compiled and installed $module"
    ;;
remove)
    [[ "$(id -u)" == 0 ]] || fatal "requires authorisation"
    protected_module
    [[ -d /System && -d "$destination" ]] || fatal "filesystem directory absent"
    safe_directory /System
    safe_directory "$destination"
    [[ -e "$module" ]] || { echo "exFAT.so already absent"; exit 0; }
    [[ -f "$module" && ! -L "$module" && "$(stat -c %u "$module")" == 0 ]] ||
        fatal "unsafe existing module"
    verify_module "$module"
    rm -f -- "$module"
    echo "Removed $module"
    ;;
esac

#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# FAT16 selector/compiler ONLY. Never installs packages, kernel modules or EFI.
set -euo pipefail
export LC_ALL=C

source_dir=/usr/lib/filesystems/fat16-source
destination=/System/Filesystems
module="$destination/FAT16.so"
compiler=/usr/bin/cc
readelf=/usr/bin/readelf

fatal() { echo "FAT16: $*" >&2; exit 1; }
[[ $# -eq 1 ]] || fatal "usage: fat16-module-helper install|remove|status"

safe_directory() {
    [[ -d "$destination" && ! -L /System && ! -L "$destination" ]] ||
        fatal "expected a real /System/Filesystems directory"
    [[ "$(stat -c %u "$destination")" == 0 ]] ||
        fatal "module directory must be owned by root"
    # Refuse a directory writable by others.
    mode=$(stat -c %a "$destination")
    (( (8#$mode & 8#022) == 0 )) ||
        fatal "module directory must not be group/world writable"
}
safe_file() {
    [[ -f "$module" && ! -L "$module" ]] ||
        fatal "FAT16.so is not a regular file"
    [[ "$(stat -c %u "$module")" == 0 ]] ||
        fatal "FAT16.so is not root owned"
}
case "$1" in
    status)
        [[ -d "$destination" && ! -L "$destination" && ! -L /System ]] ||
            exit 3
        safe_directory
        [[ -e "$module" ]] || exit 3
        safe_file
        [[ -x "$readelf" ]] || fatal "readelf not available"
        "$readelf" -h "$module" | grep 'Type:.*DYN' >/dev/null ||
            fatal "FAT16.so is not a shared ELF module"
        "$readelf" -Ws "$module" |
            grep 'infiltrator_filesystem_module_v1 ||
            fatal "FAT16 ABI entry point is missing"
        printf 'installed\n'
        ;;
    install)
        [[ $(id -u) -eq 0 ]] || fatal "requires authorisation"
        [[ -x "$compiler" ]] || fatal "C compiler missing; no packages were installed"
        [[ -x "$readelf" ]] || fatal "readelf missing; no packages were installed"
        [[ -f "$source_dir/fat16_module.c" &&
           -f "$source_dir/fat16_module.h" &&
           ! -L "$source_dir/fat16_module.c" &&
           ! -L "$source_dir/fat16_module.h" ]] ||
            fatal "packaged FAT16 module source missing"
        [[ ! -L /System && ! -L "$destination" ]] ||
            fatal "symlinked module directory is not permitted"
        if [[ ! -d /System ]]; then mkdir -m 0755 /System; fi
        if [[ ! -d "$destination" ]]; then mkdir -m 0755 "$destination"; fi
        safe_directory
        [[ ! -L "$module" ]] || fatal "existing FAT16 module is a symlink"
        temporary=$(mktemp "$destination/.FAT16.so.XXXXXXXX") ||
            fatal "cannot stage FAT16 module"
        trap 'rm -f -- "$temporary"' EXIT
        "$compiler" -std=c11 -O2 -fPIC -fno-stack-protector \
            -fno-builtin -ffreestanding -fvisibility=hidden \
            -nostdlib -shared -Wl,-z,defs -Wl,-z,relro -Wl,-z,now \
            -I "$source_dir" -o "$temporary" "$source_dir/fat16_module.c" ||
            fatal "FAT16 compilation failed; previous module unchanged"
        "$readelf" -h "$temporary" | grep 'Type:.*DYN' >/dev/null ||
            fatal "FAT16 output is not an ELF shared module"
        "$readelf" -Ws "$temporary" |
            grep -q 'infiltrator_filesystem_module_v1$' ||
            fatal "compiled FAT16 module lacks the expected interface"
        chmod 0644 "$temporary"
        chown root:root "$temporary"
        mv -fT -- "$temporary" "$module"
        trap - EXIT
        printf 'Compiled and installed %s\n' "$module"
        ;;
    remove)
        [[ $(id -u) -eq 0 ]] || fatal "requires authorisation"
        safe_directory
        [[ -e "$module" ]] || { printf 'FAT16 already absent\n'; exit 0; }
        safe_file
        rm -f -- "$module"
        printf 'Removed %s\n' "$module"
        ;;
    *)
        fatal "usage: fat16-module-helper install|remove|status"
        ;;
esac
 >/dev/null ||
            fatal "FAT16 ABI entry point is missing"
        printf 'installed\n'
        ;;
    install)
        [[ $(id -u) -eq 0 ]] || fatal "requires authorisation"
        [[ -x "$compiler" ]] || fatal "C compiler missing; no packages were installed"
        [[ -x "$readelf" ]] || fatal "readelf missing; no packages were installed"
        [[ -f "$source_dir/fat16_module.c" &&
           -f "$source_dir/fat16_module.h" &&
           ! -L "$source_dir/fat16_module.c" &&
           ! -L "$source_dir/fat16_module.h" ]] ||
            fatal "packaged FAT16 module source missing"
        [[ ! -L /System && ! -L "$destination" ]] ||
            fatal "symlinked module directory is not permitted"
        if [[ ! -d /System ]]; then mkdir -m 0755 /System; fi
        if [[ ! -d "$destination" ]]; then mkdir -m 0755 "$destination"; fi
        safe_directory
        [[ ! -L "$module" ]] || fatal "existing FAT16 module is a symlink"
        temporary=$(mktemp "$destination/.FAT16.so.XXXXXXXX") ||
            fatal "cannot stage FAT16 module"
        trap 'rm -f -- "$temporary"' EXIT
        "$compiler" -std=c11 -O2 -fPIC -fno-stack-protector \
            -fno-builtin -ffreestanding -fvisibility=hidden \
            -nostdlib -shared -Wl,-z,defs -Wl,-z,relro -Wl,-z,now \
            -I "$source_dir" -o "$temporary" "$source_dir/fat16_module.c" ||
            fatal "FAT16 compilation failed; previous module unchanged"
        "$readelf" -h "$temporary" | grep 'Type:.*DYN' >/dev/null ||
            fatal "FAT16 output is not an ELF shared module"
        "$readelf" -Ws "$temporary" |
            grep 'infiltrator_filesystem_module_v1 ||
            fatal "compiled FAT16 module lacks the expected interface"
        chmod 0644 "$temporary"
        chown root:root "$temporary"
        mv -fT -- "$temporary" "$module"
        trap - EXIT
        printf 'Compiled and installed %s\n' "$module"
        ;;
    remove)
        [[ $(id -u) -eq 0 ]] || fatal "requires authorisation"
        safe_directory
        [[ -e "$module" ]] || { printf 'FAT16 already absent\n'; exit 0; }
        safe_file
        rm -f -- "$module"
        printf 'Removed %s\n' "$module"
        ;;
    *)
        fatal "usage: fat16-module-helper install|remove|status"
        ;;
esac
 >/dev/null ||
            fatal "compiled FAT16 module lacks the expected interface"
        chmod 0644 "$temporary"
        chown root:root "$temporary"
        mv -fT -- "$temporary" "$module"
        trap - EXIT
        printf 'Compiled and installed %s\n' "$module"
        ;;
    remove)
        [[ $(id -u) -eq 0 ]] || fatal "requires authorisation"
        safe_directory
        [[ -e "$module" ]] || { printf 'FAT16 already absent\n'; exit 0; }
        safe_file
        rm -f -- "$module"
        printf 'Removed %s\n' "$module"
        ;;
    *)
        fatal "usage: fat16-module-helper install|remove|status"
        ;;
esac

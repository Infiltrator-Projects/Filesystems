#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Compile/install only the standalone EXT2.so; no APT, DKMS, .ko, EFI,
# filesystem formatting, block-device opening or implicit disk writes.
set -euo pipefail
export LC_ALL=C

self_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)
app_dir=$(cd -- "$self_dir/.." && pwd -P)
source_dir="$self_dir/../Sources/ext2"
destination=/System/Filesystems
module="$destination/EXT2.so"
compiler=/usr/bin/cc
elf_checker=/usr/bin/readelf

fatal() { echo "EXT2: $*" >&2; exit 1; }
[[ $# -eq 1 ]] || fatal "usage: ext2-module-helper install|remove|status"

safe_directory() {
    for directory in /System "$destination"; do
        [[ -d "$directory" && ! -L "$directory" ]] ||
            fatal "expected a real module directory"
        [[ "$(stat -c %u -- "$directory")" == 0 ]] ||
            fatal "module directory is not root owned"
        local mode
        mode=$(stat -c %a -- "$directory")
        (( (8#$mode & 8#022) == 0 )) ||
            fatal "module directory writable by another user"
    done
}

safe_file() {
    [[ -f "$module" && ! -L "$module" ]] ||
        fatal "EXT2.so is not a regular file"
    [[ "$(stat -c %u -- "$module")" == 0 ]] ||
        fatal "EXT2.so must be root owned"
    local mode
    mode=$(stat -c %a -- "$module")
    (( (8#$mode & 8#022) == 0 )) ||
        fatal "EXT2.so writable by another user"
}

check_module() {
    local file="$1"
    [[ -x "$elf_checker" ]] || fatal "readelf unavailable"
    "$elf_checker" -h "$file" | grep -E 'Type:.*DYN' >/dev/null ||
        fatal "not an ELF shared object"
    "$elf_checker" -Ws "$file" |
        grep -F 'infiltrator_filesystem_module_v1' >/dev/null ||
        fatal "missing module ABI entry point"
    "$elf_checker" -p .ifsmeta "$file" 2>/dev/null |
        grep -F 'IFS-MODULE:ext2:1' >/dev/null ||
        fatal "incorrect EXT2 format identity"
}

safe_source_tree() {
    for dir in "$(dirname -- "$app_dir")" "$app_dir" "$self_dir" \
               "$app_dir/Sources" "$source_dir" \
               "$source_dir/module" "$source_dir/core"; do
        [[ -d "$dir" && ! -L "$dir" ]] ||
            fatal "application source directory missing or symlinked: $dir"
        [[ "$(stat -c %u -- "$dir")" == 0 ]] ||
            fatal "source directory must be root owned: $dir"
        local mode
        mode=$(stat -c %a -- "$dir")
        (( (8#$mode & 8#022) == 0 )) ||
            fatal "source directory writable by another user: $dir"
    done
    for src in module/ext2_module.c module/ext2_module.h \
               core/ext2_core.c core/ext2_core.h \
               core/ext2_engine.c core/ext2_engine.h; do
        local file="$source_dir/$src"
        [[ -f "$file" && ! -L "$file" ]] ||
            fatal "missing or symlinked packaged source: $src"
        [[ "$(stat -c %u -- "$file")" == 0 ]] ||
            fatal "source file must be root owned: $file"
        local mode
        mode=$(stat -c %a -- "$file")
        (( (8#$mode & 8#022) == 0 )) ||
            fatal "source file writable by another user: $file"
    done
}

case "$1" in
    status)
        [[ -d "$destination" && ! -L "$destination" && ! -L /System ]] ||
            exit 3
        safe_directory
        [[ -e "$module" ]] || exit 3
        safe_file
        check_module "$module"
        printf 'installed\n'
        ;;
    install)
        [[ $(id -u) -eq 0 ]] || fatal "requires authorisation"
        safe_source_tree
        [[ -x "$compiler" ]] || fatal "C compiler missing"
        [[ -x "$elf_checker" ]] || fatal "readelf missing"
        [[ ! -L /System && ! -L "$destination" ]] ||
            fatal "symlinked module directory refused"
        if [[ ! -d /System ]]; then mkdir -m 0755 /System; fi
        if [[ ! -d "$destination" ]]; then mkdir -m 0755 "$destination"; fi
        safe_directory
        [[ ! -L "$module" ]] || fatal "existing EXT2 module is a symlink"
        temporary=$(mktemp "$destination/.EXT2.so.XXXXXXXX") ||
            fatal "cannot stage EXT2.so"
        trap 'rm -f -- "$temporary"' EXIT
        "$compiler" -std=c11 -O2 -fPIC -fno-stack-protector \
            -fno-builtin -ffreestanding -fvisibility=hidden \
            -nostdlib -shared -Wl,-z,defs -Wl,-z,relro -Wl,-z,now \
            -I "$source_dir/module" -I "$source_dir/core" \
            -o "$temporary" \
            "$source_dir/module/ext2_module.c" \
            "$source_dir/core/ext2_core.c" \
            "$source_dir/core/ext2_engine.c" ||
            fatal "compilation failed; old module unchanged"
        check_module "$temporary"
        chmod 0644 "$temporary"
        chown root:root "$temporary"
        mv -fT -- "$temporary" "$module"
        trap - EXIT
        printf 'Compiled and installed %s\n' "$module"
        ;;
    remove)
        [[ $(id -u) -eq 0 ]] || fatal "requires authorisation"
        safe_directory
        [[ -e "$module" ]] ||
            { printf 'EXT2 already absent\n'; exit 0; }
        safe_file
        rm -f -- "$module"
        printf 'Removed %s\n' "$module"
        ;;
    *)
        fatal "usage: ext2-module-helper install|remove|status"
        ;;
esac

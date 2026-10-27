#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Only run on disposable GitHub-hosted CI runners; does not touch block devices.
set -euo pipefail

if [[ "${GITHUB_ACTIONS:-}" != true ||
      "${RUNNER_ENVIRONMENT:-}" != github-hosted ]]; then
    echo 'Refusing EXT2 /System smoke test outside disposable GitHub-hosted CI.' >&2
    exit 1
fi

app_dir=/Programs/Filesystems
helper="$app_dir/Helpers/ext2-module-helper"
source_dir="$app_dir/Sources/ext2"
core_dir="$source_dir/core"
module_dir="$source_dir/module"
module=/System/Filesystems/EXT2.so
[[ ! -e "$helper" && ! -e "$module" && ! -d "$app_dir" ]] ||
  { echo "Pre-existing EXT2 test path; refusing replacement" >&2; exit 1; }

cleanup() {
    sudo "$helper" remove >/dev/null 2>&1 || :
    sudo rm -f "$helper" \
        "$module_dir/ext2_module.c" "$module_dir/ext2_module.h" \
        "$core_dir/ext2_engine.c" "$core_dir/ext2_engine.h" \
        "$core_dir/ext2_core.c" "$core_dir/ext2_core.h"
    sudo rmdir "$module_dir" "$core_dir" "$source_dir" >/dev/null 2>&1 || :
    sudo rmdir "$app_dir/Sources" "$app_dir/Helpers" "$app_dir" /Programs >/dev/null 2>&1 || :
}
trap cleanup EXIT

sudo install -d -m 0755 "$app_dir/Helpers" "$core_dir" "$module_dir"
for file in ext2_module.c ext2_module.h; do
    sudo install -m 0644 "native/filesystems/ext2/module/$file" "$module_dir/"
done
for file in ext2_engine.c ext2_engine.h ext2_core.c ext2_core.h; do
    sudo install -m 0644 "native/filesystems/ext2/core/$file" "$core_dir/"
done
sudo install -m 0755 packaging/linux/ext2-module-helper.sh "$helper"

sudo "$helper" install
[[ -f "$module" ]] || { echo "missing compiled EXT2.so" >&2; exit 1; }
[[ "$(sudo "$helper" status)" == installed ]] ||
  { echo "installed module not detected" >&2; exit 1; }
if readelf -d "$module" | grep 'NEEDED' >/dev/null; then
    echo "EXT2.so has host-linked library dependencies" >&2
    exit 1
fi
readelf -Ws "$module" | grep 'infiltrator_filesystem_module_v1$' >/dev/null

sudo "$helper" remove
[[ ! -e "$module" ]] || { echo "EXT2 removal failed" >&2; exit 1; }

# A binary for another format is never an acceptable replacement.
foreign=$(find build-desktop -name "FAT16.so" -type f -print -quit)
[[ -n "$foreign" ]] || { echo "missing foreign module for identity test" >&2; exit 1; }
sudo install -m 0644 "$foreign" "$module"
if sudo "$helper" status >/dev/null 2>&1; then
    echo "EXT2 accepted a mismatched module identity" >&2
    exit 1
fi
sudo "$helper" remove


# The copied prebuilt module and a module compiled by Install are discovered
# through the exact same directory; neither installs a second filesystem.
prebuilt=$(find build-desktop -name EXT2.so -type f -print -quit)
[[ -n "$prebuilt" ]] || { echo "prebuilt EXT2.so missing" >&2; exit 1; }
sudo install -m 0644 "$prebuilt" "$module"
[[ "$(sudo "$helper" status)" == installed ]] ||
  { echo "precompiled EXT2.so was not recognised" >&2; exit 1; }
sudo "$helper" remove

echo 'EXT2-SELECT-COMPILE-COPY-AND-DRAG-PASS'

#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Only run on disposable GitHub-hosted CI runners; does not touch block devices.
set -euo pipefail

if [[ "${GITHUB_ACTIONS:-}" != true ||
      "${RUNNER_ENVIRONMENT:-}" != github-hosted ]]; then
    echo 'Refusing FAT16 /System smoke test outside disposable GitHub-hosted CI.' >&2
    exit 1
fi

app_dir=/Programs/Filesystems
helper="$app_dir/Helpers/fat16-module-helper"
source_dir="$app_dir/Sources/fat16"
module=/System/Filesystems/FAT16.so
[[ ! -e "$helper" && ! -e "$module" && ! -d "$app_dir" ]] ||
  { echo "Pre-existing FAT16 test path; refusing replacement" >&2; exit 1; }

cleanup() {
    sudo "$helper" remove >/dev/null 2>&1 || :
    sudo rm -f "$helper" "$source_dir/fat16_module.c" "$source_dir/fat16_module.h" "$source_dir/classic_fat_module.c" "$source_dir/classic_fat_module.h"
    sudo rmdir "$source_dir" >/dev/null 2>&1 || :
    sudo rmdir "$app_dir/Sources" "$app_dir/Helpers" "$app_dir" /Programs >/dev/null 2>&1 || :
}
trap cleanup EXIT

sudo install -d -m 0755 "$app_dir/Helpers" "$source_dir"
sudo install -m 0644 native/filesystems/fat/module/fat16_module.c "$source_dir"
sudo install -m 0644 native/filesystems/fat/module/fat16_module.h "$source_dir"
sudo install -m 0644 native/filesystems/fat/module/classic_fat_module.c "$source_dir"
sudo install -m 0644 native/filesystems/fat/module/classic_fat_module.h "$source_dir"
sudo install -m 0755 packaging/linux/fat16-module-helper.sh "$helper"

sudo "$helper" install
[[ -f "$module" ]] || { echo "missing compiled FAT16.so" >&2; exit 1; }
[[ "$(sudo "$helper" status)" == installed ]] ||
  { echo "installed module not detected" >&2; exit 1; }
if readelf -d "$module" | grep 'NEEDED' >/dev/null; then
    echo "FAT16.so has host-linked library dependencies" >&2
    exit 1
fi
# Both exports and both embedded versioned identifiers belong to this format.
readelf -Ws "$module" | grep -F 'infiltrator_filesystem_module_v1' >/dev/null
readelf -Ws "$module" | grep -F 'infiltrator_filesystem_module_v2' >/dev/null
readelf -p .ifsmeta "$module" | grep -F 'IFS-MODULE:fat16:1' >/dev/null
readelf -p .ifsmeta "$module" | grep -F 'IFS-MODULE:fat16:2' >/dev/null

sudo "$helper" remove
[[ ! -e "$module" ]] || { echo "FAT16 removal failed" >&2; exit 1; }

# Renaming a different FAT family module must never pass format validation.
foreign=$(find build-desktop -name "FAT12.so" -type f -print -quit)
[[ -n "$foreign" ]] || { echo "missing foreign module" >&2; exit 1; }
sudo install -m 0644 "$foreign" "$module"
if sudo "$helper" status >/dev/null 2>&1; then
    echo "FAT16 accepted a mismatched module identity" >&2
    exit 1
fi
sudo "$helper" remove

# A manually dropped prebuilt file must be recognised as the same format.
prebuilt=$(find build-desktop -name "FAT16.so" -type f -print -quit)
[[ -n "$prebuilt" ]] || { echo "prebuilt FAT16.so missing" >&2; exit 1; }
sudo install -m 0644 "$prebuilt" "$module"
[[ "$(sudo "$helper" status)" == installed ]] ||
    { echo "precompiled FAT16.so not recognised" >&2; exit 1; }
readelf -Ws "$module" | grep -F 'infiltrator_filesystem_module_v2' >/dev/null
readelf -p .ifsmeta "$module" | grep -F 'IFS-MODULE:fat16:2' >/dev/null
sudo "$helper" remove
[[ ! -e "$module" ]] || { echo "FAT16 final removal failed" >&2; exit 1; }

echo 'FAT16-SELECT-COMPILE-COPY-AND-DRAG-PASS'

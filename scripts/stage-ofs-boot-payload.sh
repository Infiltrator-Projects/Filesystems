#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Stage Linux EFI-stub boot payloads on an ALREADY MOUNTED OFS partition.
# This script cannot format, partition, mount, alter the ESP or write NVRAM.
set -euo pipefail
fail() { echo "OFS staging: $*" >&2; exit 1; }
if [[ $# != 4 && $# != 5 ]] || [[ $1 != --mount || $3 != --root-uuid ]]; then
  echo "Usage: $0 --mount /mnt/verified-ofs --root-uuid UUID [--confirm]" >&2
  exit 2
fi
mount_dir=$2
root_uuid=$4
confirm=no
if [[ $# == 5 ]]; then
  [[ $5 == --confirm ]] || fail "unknown option"
  confirm=yes
fi
[[ $(uname -m) == x86_64 ]] || fail "only x86_64 kernels are in scope"
[[ $mount_dir == /* && -d $mount_dir && ! -L $mount_dir ]] ||
  fail "provide an absolute, existing, non-symlink mount point"
mountpoint -q -- "$mount_dir" || fail "not a mounted filesystem"
[[ $(findmnt -n -o FSTYPE --mountpoint "$mount_dir") == ofs ]] ||
  fail "mounted device is not our native OFS filesystem"
mount_opts=$(findmnt -n -o OPTIONS --mountpoint "$mount_dir")
[[ ,$mount_opts, == *,rw,* ]] || fail "OFS must already be mounted read/write"
actual_root_uuid=$(findmnt -n -o UUID --target /)
[[ -n $actual_root_uuid && $root_uuid == "$actual_root_uuid" ]] ||
  fail "root UUID must match this installation's Linux root, not the OFS card"
kernel_release=$(uname -r)
kernel="/boot/vmlinuz-$kernel_release"
initrd="/boot/initrd.img-$kernel_release"
module="/lib/modules/$kernel_release/updates/infiltrator/ofs.ko"
for source in "$kernel" "$initrd" "$module"; do
  [[ -f $source && -s $source ]] || fail "required file missing: $source"
  [[ $(stat -c %s -- "$source") -lt 4294967296 ]] ||
    fail "file is too large for this experimental OFS EFI reader: $source"
done
command -v modinfo >/dev/null || fail "modinfo is required"
read -r mod_release _ <<< "$(modinfo -F vermagic "$module")"
[[ $mod_release == "$kernel_release" ]] || fail "ofs.ko kernel ABI mismatch"
target="$mount_dir/boot"
[[ ! -L $target && ( ! -e $target || -d $target ) ]] ||
  fail "/boot on OFS must be a directory, not a link or file"
config=$(printf '"OFS Linux test" "root=UUID=%s ro initrd=\\boot\\initrd.img"\n' "$root_uuid")
for name in vmlinuz initrd.img ofs.ko refind_linux.conf; do
  [[ ! -L $target/$name && ! -e $target/$name.ofstmp ]] ||
    fail "refusing pre-existing staging file or symlink: $target/$name"
done
if [[ -d $target ]]; then
  for pair in "vmlinuz:$kernel" "initrd.img:$initrd" "ofs.ko:$module"; do
    IFS=: read -r name source <<< "$pair"
    if [[ -e $target/$name ]]; then
      cmp -s -- "$source" "$target/$name" ||
        fail "refusing to overwrite nonmatching $target/$name"
    fi
  done
  if [[ -e $target/refind_linux.conf ]]; then
    [[ $(cat -- "$target/refind_linux.conf") == "$config" ]] ||
      fail "refusing to overwrite nonmatching boot configuration"
  fi
fi
printf 'OFS: %s\nKernel: %s\nInitramfs: %s\nModule: %s\nRoot UUID: %s\n' \
  "$mount_dir" "$kernel" "$initrd" "$module" "$root_uuid"
if [[ $confirm != yes ]]; then
  echo "Preflight passed; no writes. Rerun with --confirm to stage."
  exit 0
fi
mkdir -p -- "$target"
copy_checked() {
  local src=$1 dst=$2 tmp
  tmp="$dst.ofstmp"
  if [[ -e $dst ]]; then
    cmp -s -- "$src" "$dst" || fail "destination changed: $dst"
  else
    [[ ! -e $tmp ]] || fail "occupied staging filename: $tmp"
    cp -- "$src" "$tmp"
    cmp -s -- "$src" "$tmp" || fail "copy verification failed: $tmp"
    mv -n -- "$tmp" "$dst"
    cmp -s -- "$src" "$dst" || fail "rename verification failed: $dst"
  fi
  echo "Verified: $dst"
}
copy_checked "$kernel" "$target/vmlinuz"
copy_checked "$initrd" "$target/initrd.img"
copy_checked "$module" "$target/ofs.ko"
config_temp=$(mktemp)
trap 'rm -f -- "$config_temp"' EXIT
printf '%s\n' "$config" > "$config_temp"
copy_checked "$config_temp" "$target/refind_linux.conf"
sync -f -- "$target/vmlinuz" "$target/initrd.img" "$target/ofs.ko" "$target/refind_linux.conf"
echo "OFS boot payload staged. The EFI driver must be installed separately."

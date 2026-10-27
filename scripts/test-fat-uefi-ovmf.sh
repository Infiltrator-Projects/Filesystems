#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Exercise only disposable disk images under QEMU/OVMF. Firmware's built-in
# FAT driver is explicitly disconnected; the probe invokes our driver's own
# binding instance and checks that the other FAT variant is rejected.
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")/.." && pwd -P)
out=${FAT_EFI_TEST_OUTPUT:-${RUNNER_TEMP:-/tmp}/fat-efi-qualification}
[[ ! -e "$out" ]] || { echo "Refusing existing test output directory: $out" >&2; exit 1; }
mkdir -p "$out"
source "$root/scripts/gnu-efi-toolchain.sh"
for cmd in qemu-system-x86_64 mkfs.fat fsck.fat mcopy mmd sgdisk timeout python3 gcc ld objcopy; do
    command -v "$cmd" >/dev/null
done
code=/usr/share/OVMF/OVMF_CODE_4M.fd
vars=/usr/share/OVMF/OVMF_VARS_4M.fd
test -s "$code" && test -s "$vars"
for bits in 12 16; do
    driver="$root/build-fat-efi/FAT$bits.efi"
    test -f "$driver" && test ! -L "$driver" && test -s "$driver"
    case_dir="$out/fat$bits"
    mkdir -p "$case_dir"
    gcc -Os -std=gnu11 -fpic -ffreestanding -fno-builtin -fshort-wchar \
      -fno-stack-protector -fno-strict-aliasing -mno-red-zone \
      -fno-tree-loop-distribute-patterns -Wall -Wextra -Werror \
      -DEFIX64 '-DEFIAPI=__attribute__((ms_abi))' -DEFI_FUNCTION_WRAPPER \
      -DPROBE_FAT_BITS="$bits" -I"$gnu_efi_include" -I"$gnu_efi_include/x86_64" \
      -I"$gnu_efi_include/protocol" -c "$root/tests/fat_uefi_rw_probe.c" -o "$case_dir/probe.o"
    ld -T "$ldscript" -shared -Bsymbolic -nostdlib -znocombreloc -zdefs \
      "$crt" "$case_dir/probe.o" "$libefi" "$libgnuefi" \
      "$(gcc -print-libgcc-file-name)" -o "$case_dir/probe.so"
    objcopy --output-target=efi-app-x86_64 \
      -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rodata \
      -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
      "$case_dir/probe.so" "$case_dir/BOOTX64.EFI"
    esp="$case_dir/esp.img"
    image12="$case_dir/fat12.img"
    image16="$case_dir/fat16.img"
    disk="$case_dir/disk.img"
    truncate -s 32M "$esp"
    truncate -s 1440K "$image12"
    truncate -s 8M "$image16"
    mkfs.fat -F 16 -I "$esp" >/dev/null
    mkfs.fat -F 12 -I "$image12" >/dev/null
    mkfs.fat -F 16 -I -s 1 "$image16" >/dev/null
    mmd -i "$esp" ::/EFI ::/EFI/BOOT
    mcopy -i "$esp" "$case_dir/BOOTX64.EFI" ::/EFI/BOOT/BOOTX64.EFI
    mcopy -i "$esp" "$driver" "::/FAT$bits.efi"
    truncate -s 128M "$disk"
    sgdisk --clear --new=1:2048:67583 --typecode=1:ef00 \
      --new=2:67584:70463 --typecode=2:8300 \
      --new=3:71680:88063 --typecode=3:8300 "$disk" >/dev/null
    dd if="$esp" of="$disk" bs=512 seek=2048 conv=notrunc status=none
    dd if="$image12" of="$disk" bs=512 seek=67584 conv=notrunc status=none
    dd if="$image16" of="$disk" bs=512 seek=71680 conv=notrunc status=none
    cp "$vars" "$case_dir/vars.fd"
    set +e
    timeout -k 5s 90s qemu-system-x86_64 \
      -machine q35,accel=tcg -m 256 -display none -monitor none \
      -serial "file:$case_dir/serial.log" -nic none \
      -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
      -drive "if=pflash,format=raw,readonly=on,file=$code" \
      -drive "if=pflash,format=raw,file=$case_dir/vars.fd" \
      -drive "if=ide,format=raw,file=$disk" -boot order=c
    status=$?
    set -e
    cat "$case_dir/serial.log" || true
    [[ $status == 33 ]] || { echo "FAT$bits QEMU exit $status, expected 33" >&2; exit 1; }
    grep -Fq 'FAT_EFI_PASS read write rename delete flush rebind reject' "$case_dir/serial.log"
    dd if="$disk" of="$case_dir/after12.img" bs=512 skip=67584 count=2880 status=none
    dd if="$disk" of="$case_dir/after16.img" bs=512 skip=71680 count=16384 status=none
    fsck.fat -n "$case_dir/after12.img"
    fsck.fat -n "$case_dir/after16.img"
    if [[ $bits == 12 ]]; then
        cmp "$image16" "$case_dir/after16.img"
    else
        cmp "$image12" "$case_dir/after12.img"
    fi
    mcopy -i "$case_dir/after$bits.img" '::/RESULT/Renamed Long Name.bin' "$case_dir/readback.bin"
    if mdir -i "$case_dir/after$bits.img" '::/RESULT/DELETE.TXT' >/dev/null 2>&1; then
        echo "Deleted EFI file still exists" >&2; exit 1
    fi
    python3 - "$case_dir/readback.bin" <<'PY'
from pathlib import Path
import sys
actual=Path(sys.argv[1]).read_bytes()
assert actual == bytes((i*17+23)&255 for i in range(4097)), 'external FAT reader mismatch'
print('FAT EFI independent readback verified',sys.argv[1])
PY
done
echo 'FAT12-FAT16-EFI-OVMF-RUNTIME-PASS'

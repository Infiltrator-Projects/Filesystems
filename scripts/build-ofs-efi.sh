#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Build Filesystems-owned OFS UEFI driver; never modify EFI partitions or NVRAM.
set -euo pipefail
root="$(cd -- "$(dirname -- "$0")/.." && pwd)"
test "$(uname -m)" = x86_64
tools="$(dpkg -L gnu-efi)"
crt="$(grep '/crt0-efi-x86_64\.o$' <<< "$tools" | head -n1)"
ldscript="$(grep '/elf_x86_64_efi\.lds$' <<< "$tools" | head -n1)"
libefi="$(grep '/libefi\.a$' <<< "$tools" | head -n1)"
libgnuefi="$(grep '/libgnuefi\.a$' <<< "$tools" | head -n1)"
for item in "$crt" "$ldscript" "$libefi" "$libgnuefi"; do test -s "$item"; done
out="$(realpath -m -- "$(pwd)/$(basename -- "$root")-ofs-efi")"
if test "$#" -ge 1; then out="$(realpath -m -- "$1")"; fi
mkdir -p "$out"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
ofs="$root/native/filesystems/ofs"
for filename in uefi_driver reader; do
    gcc -Os -std=gnu11 -fpic -ffreestanding -fno-builtin -fshort-wchar \
      -fno-stack-protector -fno-strict-aliasing -mno-red-zone \
      -fno-tree-loop-distribute-patterns -Wall -Wextra -Werror \
      -DEFIX64 '-DEFIAPI=__attribute__((ms_abi))' -DEFI_FUNCTION_WRAPPER \
      -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol \
      -I"$ofs/efi" -I"$ofs/core" \
      -c "$ofs/efi/$filename.c" -o "$work/$filename.o"
done
for filename in ofs_core ofs_primitives; do
    gcc -Os -std=gnu11 -fpic -ffreestanding -fno-builtin -fshort-wchar \
      -fno-stack-protector -fno-strict-aliasing -mno-red-zone \
      -fno-tree-loop-distribute-patterns -Wall -Wextra -Werror \
      -DEFIX64 '-DEFIAPI=__attribute__((ms_abi))' -DEFI_FUNCTION_WRAPPER \
      -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol \
      -I"$ofs/efi" -I"$ofs/core" \
      -c "$ofs/core/$filename.c" -o "$work/$filename.o"
done
gcc_lib="$(gcc -print-libgcc-file-name)"
ld -T "$ldscript" -shared -Bsymbolic -nostdlib -znocombreloc -zdefs \
  "$crt" "$work/uefi_driver.o" "$work/reader.o" "$work/ofs_core.o" \
  "$work/ofs_primitives.o" "$libefi" "$libgnuefi" "$gcc_lib" \
  -o "$work/OFS_x64.so"
objcopy --output-target=efi-bsdrv-x86_64 \
  -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rodata \
  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc \
  "$work/OFS_x64.so" "$out/OFS_x64.efi"
test -s "$out/OFS_x64.efi"
python3 - "$out/OFS_x64.efi" <<'PY'
import struct, sys
from pathlib import Path
p = Path(sys.argv[1])
b = p.read_bytes()
assert len(b) > 512 and b[:2] == b'MZ'
i = struct.unpack_from('<I', b, 0x3c)[0]
assert b[i:i+4] == b'PE\x00\x00'
assert struct.unpack_from('<H', b, i+4)[0] == 0x8664
assert struct.unpack_from('<H', b, i+24)[0] == 0x20b
assert struct.unpack_from('<H', b, i+24+68)[0] == 11
print('Filesystems OFS: x86-64 PE32+ EFI boot-service driver OK')
PY
sha256sum "$out/OFS_x64.efi"
echo "Generated: $out/OFS_x64.efi"

#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# All writes remain inside private disposable regular image files.
set -euo pipefail
for tool in mkfs.fat fsck.fat mcopy mke2fs debugfs e2fsck python3 cmp truncate; do
  command -v "$tool" >/dev/null || { echo "Missing prerequisite: $tool" >&2; exit 1; }
done
fat_tester=$1
ext2_tester=$2
dir=$(mktemp -d /tmp/ifs-fat16-ext2-XXXXXX)
trap 'rm -rf -- "$dir"' EXIT
python3 - "$dir" <<'PY'
import pathlib,sys
root=pathlib.Path(sys.argv[1])
original=b'A'*16384
(root/'original.bin').write_bytes(original)
for fs,marker in [('fat16',b'FAT16-RW-OK'),('ext2',b'EXT2-RW-OK!')]:
    expected=bytearray(original)
    expected[2046:2046+len(marker)]=marker
    (root/f'expected-{fs}.bin').write_bytes(expected)
PY
fat_image="$dir/fat16.img"
truncate -s 64M "$fat_image"
mkfs.fat -F 16 -s 4 -n HALTEST "$fat_image" >/dev/null
mcopy -i "$fat_image" "$dir/original.bin" ::ROUND.BIN
"$fat_tester" "$fat_image"
mcopy -i "$fat_image" ::ROUND.BIN "$dir/result-fat16.bin"
cmp "$dir/expected-fat16.bin" "$dir/result-fat16.bin"
fsck.fat -n "$fat_image" >/dev/null
echo "FAT16 independently formatted/readback/fsck: PASS"
ext2_image="$dir/ext2.img"
truncate -s 64M "$ext2_image"
mke2fs -q -F -t ext2 -b 1024 -I 128 -O filetype,sparse_super,large_file "$ext2_image"
debugfs -w -R "write $dir/original.bin /ROUND.BIN" "$ext2_image" >/dev/null 2>&1
"$ext2_tester" "$ext2_image"
debugfs -R "dump /ROUND.BIN $dir/result-ext2.bin" "$ext2_image" >/dev/null 2>&1
cmp "$dir/expected-ext2.bin" "$dir/result-ext2.bin"
e2fsck -fn "$ext2_image" >/dev/null
echo "EXT2 independently formatted/readback/fsck: PASS"
echo "FAT16-EXT2-INDEPENDENT-IMAGE-ROUNDTRIP-PASS"

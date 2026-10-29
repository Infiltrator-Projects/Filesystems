#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Real EXT2 round-trip against an independently produced disposable image.
set -euo pipefail
[[ $# -eq 1 ]] || { echo "usage: $0 ext2-tester" >&2; exit 2; }
for tool in mke2fs debugfs e2fsck python3 cmp truncate; do
  command -v "$tool" >/dev/null || { echo "Missing: $tool" >&2; exit 1; }
done
tester=$1
dir=$(mktemp -d /tmp/ifs-ext2-independent-XXXXXXXX)
trap 'rm -rf -- "$dir"' EXIT
python3 - "$dir" <<'PY'
import pathlib,sys
root=pathlib.Path(sys.argv[1])
original=b'A'*16384
(root/'original.bin').write_bytes(original)
expected=bytearray(original)
marker=b'EXT2-RW-OK!'
expected[2046:2046+len(marker)]=marker
(root/'expected.bin').write_bytes(expected)
PY
image="$dir/ext2.img"
truncate -s 64M "$image"
mke2fs -q -F -t ext2 -b 1024 -I 128 -O filetype,sparse_super,large_file "$image"
debugfs -w -R "write $dir/original.bin /ROUND.BIN" "$image" >/dev/null 2>&1
"$tester" "$image"
debugfs -R "dump /ROUND.BIN $dir/result.bin" "$image" >/dev/null 2>&1
cmp "$dir/expected.bin" "$dir/result.bin"
e2fsck -fn "$image" >/dev/null
echo "EXT2-INDEPENDENT-IMAGE-ROUNDTRIP-PASS"

#!/usr/bin/env python3
"""Fail-closed regression tests for the image-only AmigaDOS OFS formatter."""
import os
import pathlib
import shutil
import struct
import subprocess
import sys
import tempfile


def command(tool, *args, success=True):
    result = subprocess.run([str(tool), *map(str, args)], capture_output=True, text=True)
    if (result.returncode == 0) != success:
        raise AssertionError(
            f"unexpected status {result.returncode}: {args}\n"
            f"stdout={result.stdout}\nstderr={result.stderr}"
        )


def damage(image, offset):
    with image.open("r+b") as fh:
        fh.seek(offset)
        before = fh.read(1)
        fh.seek(offset)
        fh.write(bytes((before[0] ^ 0x80,)))


def test(tool, root):
    for size in (1, 32, 64):  # 64 MiB forces bitmap-extension traversal
        path = root / f"ofs-{size}.img"
        command(tool, "--create", path, size, "LD_AMIGA")
        assert path.stat().st_size == size * (1 << 20)
        command(tool, "--verify", path)
        command(tool, "--create", path, size, "LD_AMIGA", success=False)
        command(tool, "--verify", path)

    existing = root / "existing.bin"
    existing.write_bytes(b"USER DATA MUST NOT CHANGE")
    command(tool, "--create", existing, 4, success=False)
    assert existing.read_bytes() == b"USER DATA MUST NOT CHANGE"
    link = root / "existing-link"
    link.symlink_to(existing)
    command(tool, "--create", link, 4, success=False)
    command(tool, "--verify", link, success=False)
    assert existing.read_bytes() == b"USER DATA MUST NOT CHANGE"

    for args in [
        ("0",), ("1025",), ("1.5",), ("-1",),
    ]:
        command(tool, "--create", root / ("bad-" + args[0]), *args,
                success=False)
    command(tool, "--create", root / "bad-label", 4, "bad:name", success=False)
    command(tool, "--create", root / "long-label", 4, "x" * 31, success=False)
    command(tool, "--create", root / "empty-label", 4, "", success=False)

    healthy = root / "ofs-1.img"
    with healthy.open("rb") as fh:
        fh.seek((1 << 20) // 2)
        # root block is located at the centre of the volume
        fh.seek(1024 * 512)
        rootblock = fh.read(512)
    pointer = struct.unpack_from(">I", rootblock, 79 * 4)[0]
    assert 2 <= pointer < 2048
    cases = {
        "boot": 0,
        "root checksum": 1024 * 512 + 5 * 4,
        "bitmap checksum": pointer * 512,
        "bitmap bit": pointer * 512 + 8,
        "directory size": 1024 * 512 + 3 * 4,
    }
    for label, offset in cases.items():
        victim = root / f"corrupt-{label}.img"
        shutil.copyfile(healthy, victim)
        damage(victim, offset)
        command(tool, "--verify", victim, success=False)

    # No device should be accepted: creation is exclusive, verification is file-only.
    if os.path.exists("/dev/null"):
        command(tool, "--verify", "/dev/null", success=False)
        command(tool, "--create", "/dev/null", 4, success=False)


if __name__ == "__main__":
    with tempfile.TemporaryDirectory(prefix="ofs-image-test-") as tmp:
        test(pathlib.Path(sys.argv[1]), pathlib.Path(tmp))
    print("OFS image creation, extension geometry and failure-safety checks: PASS")

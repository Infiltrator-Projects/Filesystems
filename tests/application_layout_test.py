#!/usr/bin/env python3
"""Verify package source/selector paths against the native application layout.

This is a static guard; executable and copy equivalence are tested on CI runners.
"""
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parent.parent
cmake = (root / "CMakeLists.txt").read_text()
assert 'set(FILESYSTEMS_APPLICATION_ROOT "/Programs/Filesystems"' in cmake
assert 'CPACK_SET_DESTDIR ON' in cmake
for path in ["src/installer.cpp", "src/probe.cpp"]:
    source = (root / path).read_text()
    assert '#include "app_resources.hpp"' in source
    for name in ["fat16-module-helper", "ofs-module-helper", "ext2-module-helper"]:
        assert f'application_helper("{name}")' in source
        assert f'/usr/lib/filesystems/{name}' not in source
for fs in ["fat16", "ofs", "ext2", "bfs"]:
    shell = (root / f"packaging/linux/{fs}-module-helper.sh").read_text()
    assert 'source_dir="$self_dir/../Sources/' + fs + '"' in shell
    assert "/usr/lib/filesystems/" not in shell
    assert re.search(r'destination=/System/Filesystems\s', shell)
print("FILESYSTEMS-NATIVE-PROGRAM-LAYOUT-PASS")

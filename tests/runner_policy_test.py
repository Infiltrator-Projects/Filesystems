#!/usr/bin/env python3
"""Keep untrusted Filesystems CI away from the private build fleet."""
from pathlib import Path
import re
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
build = (root / ".github/workflows/build.yml").read_text()
amiga = (root / ".github/workflows/amiga-runtime-qualification.yml").read_text()
ext = (root / ".github/workflows/ext-runtime-qualification.yml").read_text()
publish = (root / ".github/workflows/apt-publication.yml").read_text()


def job(source: str, name: str) -> str:
    match = re.search(r"(?m)^  " + re.escape(name) + r":\s*$", source)
    if not match:
        raise AssertionError(f"Missing job {name}")
    remainder = source[match.end():]
    next_job = re.search(r"(?m)^  [a-z][a-z0-9-]+:\s*$", remainder)
    return remainder[:next_job.start()] if next_job else remainder


selfhost = '["self-hosted","Linux","X64","linux-native","bigbedroom"]'
hosted = '["ubuntu-24.04"]'

for name in ("linux-native-modules-6-12", "linux-amiga-v7-module-api"):
    source = job(build, name)
    assert "github.event_name == 'push'" in source, name
    assert "github.ref == 'refs/heads/main'" in source, name
    assert selfhost in source and hosted in source, name
    assert "runner.environment" in source, name
    assert 'if [[ "${{ runner.environment }}" == "github-hosted" ]]' in source, name
    assert "pkg-config --exists libelf openssl" in source, name
    assert 'kernel_tree="$RUNNER_TEMP/' in source, name
    assert 'make -C "$KERNEL_TREE"' in source, name
    assert "/tmp/linux" not in source, name
    assert source.index("sudo apt-get update") < source.index("else\n"), name

# Independent qualification and publication remain on clean hosted runners.
for source in (amiga, ext):
    assert "runs-on: ubuntu-24.04" in source
    assert "runs-on: [self-hosted" not in source
for name in ("desktop-integration", "source-boundary", "linux", "publish-release"):
    assert "runs-on: ubuntu-24.04" in job(build, name), name
for name in ("windows-manager", "windows-ext2-driver"):
    assert "runs-on: windows-latest" in job(build, name), name
assert "runs-on: ubuntu-24.04" in publish
assert "self-hosted" not in publish

print("Filesystems hybrid runner and release-isolation policy: PASS")

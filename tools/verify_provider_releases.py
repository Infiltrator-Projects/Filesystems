#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Reproduce the pinned public package/probe evidence for the complete register."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
from urllib.parse import urlsplit
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]


def download(repo, release, asset, target):
    name = asset["name"]
    if Path(name).name != name or urlsplit(repo).netloc != "github.com":
        raise ValueError(f"Invalid asset for {repo}")
    url = f"{repo}/releases/download/{release['version']}/{name}"
    sha = hashlib.sha256()
    with urlopen(url, timeout=90) as response, target.open("wb") as stream:
        while block := response.read(1024 * 1024):
            sha.update(block)
            stream.write(block)
    if sha.hexdigest() != asset["sha256"]:
        raise ValueError(f"Published digest differs for {repo}/{name}")


def qualify(record, executable):
    release = record["release"]
    with tempfile.TemporaryDirectory() as directory:
        work = Path(directory)
        for kind in ("deb", "run"):
            asset = release[kind]
            download(record["url"], release, asset, work / asset["name"])
        deb = work / release["deb"]["name"]
        root = work / "unpacked"
        subprocess.run(["dpkg-deb", "-x", deb, root], check=True)
        modules = sorted((root / "System/Filesystems").glob("*.so"))
        if [p.name for p in modules] != sorted(release["deb"]["providers"]):
            raise ValueError(f"Installed provider paths changed in {deb.name}")
        if "101" in record["status"]["Q"]:
            if len(modules) != 1:
                raise ValueError(f"Expected one production module for {deb.name}")
            symbols = subprocess.check_output(["nm", "-D", "--defined-only", modules[0]], text=True)
            if [line.split()[-1] for line in symbols.splitlines()] != ["infiltrator_filesystem_api_v2"]:
                raise ValueError(f"Unexpected exported ABI in {deb.name}")
            case = record["published_probe"]
            args = [executable, modules[0], case["format_id"], "--blank-bytes", str(case["image_bytes"])]
            result = subprocess.run(args, text=True, capture_output=True, timeout=60)
            if result.returncode:
                raise ValueError(f"{deb.name}: {result.stdout} {result.stderr}")
            return record["repository"], release["version"], "passed", result.stdout.strip()
        return record["repository"], release["version"], "published assets verified", "No qualified runtime probe recorded"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("probe", type=Path)
    args = parser.parse_args()
    records = json.loads((ROOT / "data/provider-register.json").read_text())["repositories"]
    with ThreadPoolExecutor(max_workers=4) as executor:
        jobs = [executor.submit(qualify, r, args.probe.resolve()) for r in records]
        for job in jobs:
            print(*job.result(), sep="\t", flush=True)


if __name__ == "__main__":
    main()

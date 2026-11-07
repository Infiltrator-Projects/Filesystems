#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""0.5.110 -> 0.5.116 real dpkg upgrade ONLY on an ephemeral GitHub runner.

Cannot be run on the user's Linux Mint computer or any self-hosted runner.
"""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile

OLD = ("Filesystems-0.5.110-amd64.deb", "v0.5.110",
       "92a62c46b4142183071362de60b85c0e5b951cd0766bf03695be499019ed5439")
FORMATS = (
    ("FFS", "ffs", "1.0.7",
     "7a358bb9d690fb8fc44dc4a9c2388f624225252915c32a6458e8442a8bd16200", "FFS.so"),
    ("exFAT", "exfat", "0.1.10",
     "670f97d0a1c14e9f3fdf721b9dcf2c20182cff90525b7169ba0e349bba1a7df5", "exFAT.so"),
    ("FAT12", "fat12", "0.1.13",
     "4899ccdf25948a292a7a20605c4793558ace656aeafd57e2c1aeab4fd6be0e5e", "FAT12.so"),
    ("FAT16", "fat16", "0.1.14",
     "beee0cf3af77c03f72dd9dc9decc16e3f795d75d563bb1a15642f275e6d54169", "FAT16.so"),
    ("FAT32", "fat32", "0.1.10",
     "8f25d5ea31fff9468c40b236021c2a8e3d39d7df4baf7a60e3fd4d", "FAT32.so"),
)
CANARIES = (
    "APFS.so", "BTRFS.so", "EXT2.so", "EXT3.so", "EXT4.so",
    "HFSPLUS.so", "HFS.so", "MINIX.so", "NTFS.so", "OFS.so",
    "PFS3.so", "SFS2.so", "SFS.so", "SWAP.so", "UFS1.so",
    "UFS2.so", "XFS.so", "ZFS.so",
)
SO_ROOT = Path("/System/Filesystems")


def run(*args, capture=False):
    p = subprocess.run([str(a) for a in args], text=True, check=True,
                       capture_output=capture)
    return p.stdout.strip() if capture else ""


def download(url, expected, destination):
    run("curl", "-fLSs", "--retry", "3", "-o", destination, url)
    assert hashlib.sha256(destination.read_bytes()).hexdigest() == expected
    return destination


def field(deb, name):
    return run("dpkg-deb", "-f", deb, name, capture=True)


def file_hashes():
    files = sorted(SO_ROOT.glob("*.so"))
    assert len(files) == 23, "Expected 23 isolated .so files"
    return {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in files}


def upgrade(deb):
    plan = run("sudo", "apt-get", "-s", "--no-remove",
               "--no-install-recommends", "install", deb, capture=True)
    assert not any(line.startswith(("Remv ", "Purg "))
                   for line in plan.splitlines()), plan
    run("sudo", "apt-get", "-y", "--no-remove",
        "--no-install-recommends", "install", deb)


def main():
    assert os.getenv("GITHUB_ACTIONS") == "true"
    assert os.getenv("RUNNER_OS") == "Linux"
    assert os.getenv("RUNNER_ENVIRONMENT") == "github-hosted", (
        "Refusing to touch any personal computer or self-hosted runner"
    )
    assert len(sys.argv) == 2
    assert not SO_ROOT.exists(), "Refusing to overwrite pre-existing /System"
    new = Path(sys.argv[1]).resolve()
    assert field(new, "Package") == "filesystems"
    assert field(new, "Version") == "0.5.116"
    with tempfile.TemporaryDirectory(prefix="filesystems-upgrade-qa-") as tmp:
        work = Path(tmp)
        staged = work / "new"
        run("dpkg-deb", "-x", new, staged)
        assert not (staged / "System/Filesystems").exists()
        assert not (staged / "Programs/Filesystems/Sources/fat16").exists()
        assert not (staged / "Programs/Filesystems/Sources/ofs").exists()
        new_binary = staged / "Programs/Filesystems/Filesystems"
        launcher = staged / "usr/bin/filesystems"
        desktop = staged / "usr/share/applications/org.infiltrator.Filesystems.desktop"
        assert new_binary.is_file() and launcher.is_file() and desktop.is_file()
        assert "0.5.116" in run("strings", "-a", new_binary, capture=True).splitlines()
        assert "Exec=filesystems" in desktop.read_text().splitlines()

        old = download(
            "https://github.com/Infiltrator-Projects/Filesystems/releases/download/"
            + OLD[1] + "/" + OLD[0], OLD[2], work / OLD[0]
        )
        assert field(old, "Package") == "filesystems"
        assert field(old, "Version") == "0.5.110"

        standalone = []
        for repository, slug, version, sha, soname in FORMATS:
            name = "infiltrator-filesystem-" + slug
            filename = name + "_" + version + "_amd64.deb"
            deb = download(
                "https://github.com/Infiltrator-Projects/Filesystem-" + repository
                + "/releases/download/v" + version + "/" + filename,
                sha, work / filename
            )
            assert field(deb, "Package") == name
            upgrade(deb)
            standalone.append((name, soname))
        run("sudo", "install", "-d", "-m", "0755", SO_ROOT)
        for soname in CANARIES:
            subprocess.run(
                ["sudo", "tee", str(SO_ROOT / soname)],
                input=("INDEPENDENT-CANARY-" + soname + "\n").encode(),
                stdout=subprocess.DEVNULL, check=True
            )
        baseline = file_hashes()
        for pkg, soname in standalone:
            assert run("dpkg-query", "-S", SO_ROOT / soname, capture=True) == (
                pkg + ": " + str(SO_ROOT / soname)
            )

        upgrade(old)
        assert run("dpkg-query", "-W", "-f=$" + "{Version}", "filesystems",
                   capture=True) == "0.5.110"
        assert file_hashes() == baseline, "Installing old manager changed a format"
        upgrade(new)
        assert run("dpkg-query", "-W", "-f=$" + "{Version}", "filesystems",
                   capture=True) == "0.5.116"
        assert file_hashes() == baseline, "Upgrading manager changed a format"
        for pkg, soname in standalone:
            assert run("dpkg-query", "-S", SO_ROOT / soname, capture=True) == (
                pkg + ": " + str(SO_ROOT / soname)
            )
            assert run("dpkg-query", "-W", "-f=$" + "{db:Status-Status}", pkg,
                       capture=True) == "installed"

        for path in ("/usr/bin/filesystems",
                     "/Programs/Filesystems/Filesystems",
                     "/usr/share/applications/org.infiltrator.Filesystems.desktop"):
            assert run("dpkg-query", "-S", path, capture=True).startswith("filesystems: ")
        actual = Path("/Programs/Filesystems/Filesystems")
        assert hashlib.sha256(actual.read_bytes()).hexdigest() == (
            hashlib.sha256(new_binary.read_bytes()).hexdigest()
        ), "Installed executable not the qualified package payload"
        assert "Exec=filesystems" in Path(
            "/usr/share/applications/org.infiltrator.Filesystems.desktop"
        ).read_text().splitlines()
        owned = run("dpkg-query", "-L", "filesystems", capture=True)
        assert not any(p.startswith("/System/Filesystems/") for p in owned.splitlines())
        smoke = subprocess.run(
            ["timeout", "5s", "xvfb-run", "-a", "/usr/bin/filesystems"],
            capture_output=True, text=True, timeout=20
        )
        assert smoke.returncode in (0, 124), (
            "Launcher failed", smoke.returncode, smoke.stderr[-1400:]
        )
        assert file_hashes() == baseline
        print("FILESYSTEMS-0.5.110-TO-0.5.116-DEBIAN-UPGRADE-PASS")
        print("23-INDEPENDENT-SO-HASHES-AND-5-DPKG-OWNERS-PRESERVED")
        print("FILESYSTEMS-DESKTOP-LAUNCHER-AND-EXECUTABLE-UPGRADED")


if __name__ == "__main__":
    main()

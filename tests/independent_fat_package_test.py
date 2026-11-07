#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Safely test published FAT12, FAT16 and FAT32 package independence.

Everything is downloaded into a temporary folder, and dpkg operations use a
PRIVATE --root= directory. This test must never modify host /System or disks.
"""
import hashlib
import os
from pathlib import Path
import re
import subprocess
import tempfile

# (format width, release version, published Debian asset SHA256,
#  published source revision)
RELEASES = (
    (12, "0.1.13",
     "4899ccdf25948a292a7a20605c4793558ace656aeafd57e2c1aeab4fd6be0e5e",
     "d9f970ae8b3cc436bdc616b3919ae0b25c29c067"),
    (16, "0.1.14",
     "beee0cf3af77c03f72dd9dc9decc16e3f795d75d563bb1a15642f275e6d54169",
     "6378dfa310f0ed86eca9623564a06ad1e5f3acac"),
    (32, "0.1.10",
     "8f25d5ea31fff9468c40b236021c2a8e92e95d2a8e3d39d7df4baf7a60e3fd4d",
     "3b4a8d3a551fea447a9aa43f52b81f810ee287b5"),
)


def run(*argv, capture=False):
    result = subprocess.run([str(v) for v in argv], text=True,
                            check=True, capture_output=capture)
    return result.stdout.strip() if capture else ""


def slug(item):
    return "fat" + str(item[0])


def package(item):
    return "infiltrator-filesystem-" + slug(item)


def so(item):
    return "FAT" + str(item[0]) + ".so"


def field(archive, key):
    return run("dpkg-deb", "-f", archive, key, capture=True)


def status(root, pkg):
    database = (root / "var/lib/dpkg/status").read_text()
    for stanza in database.split("\n\n"):
        if re.search(r"^Package: " + re.escape(pkg) + r"$",
                     stanza, re.MULTILINE):
            match = re.search(r"^Status: (.+)$", stanza, re.MULTILINE)
            return match.group(1) if match else ""
    return ""


def inspect(archive, item, work):
    width, _, _, revision = item
    identifier = slug(item)
    assert field(archive, "Package") == package(item)
    assert field(archive, "X-Infiltrator-Source-Revision") == revision
    assert field(archive, "Architecture") == "amd64"
    assert field(archive, "X-Infiltrator-Native-Profile") == "portable"
    assert field(archive, "Version") == item[1]

    control = work / (identifier + "-control")
    run("dpkg-deb", "-e", archive, control)
    for forbidden in ("preinst", "postinst", "prerm", "postrm", "triggers"):
        assert not (control / forbidden).exists(), (
            identifier, "unexpected maintainer script", forbidden
        )

    stage = work / (identifier + "-staging")
    run("dpkg-deb", "-x", archive, stage)
    assert not any(p.is_symlink() for p in stage.rglob("*")), identifier

    module = "System/Filesystems/" + so(item)
    pdf = "System/Resources/Documentation/Filesystem-FAT" + str(width) + ".pdf"
    kernel = "System/Kernel/Filesystems/fat" + str(width) + ".ko"
    payload = {str(p.relative_to(stage)) for p in stage.rglob("*") if p.is_file()}
    assert payload in ({module, pdf}, {module, pdf, kernel}), (
        identifier, "unexpected package payload", payload
    )

    binary, manual = stage / module, stage / pdf
    assert manual.read_bytes().startswith(b"%PDF-"), identifier
    assert hashlib.sha256(binary.read_bytes()).hexdigest() == field(
        archive, "X-Infiltrator-Resource-SHA256"
    ), identifier
    assert hashlib.sha256(manual.read_bytes()).hexdigest() == field(
        archive, "X-Infiltrator-PDF-SHA256"
    ), identifier

    header = run("readelf", "-h", binary, capture=True)
    assert "ELF64" in header and "DYN (" in header
    assert "Advanced Micro Devices X86-64" in header
    exports = run("readelf", "-W", "--dyn-syms", binary, capture=True)
    for symbol in ("infiltrator_filesystem_module_v1",
                   "infiltrator_filesystem_module_v3",
                   "infiltrator_filesystem_module_v4",
                   "infiltrator_filesystem_defrag_v1"):
        assert symbol in exports, (identifier, "missing ABI", symbol)

    markers = run("readelf", "-p", ".ifsmeta", binary, capture=True)
    assert "IFS-MODULE:" + identifier + ":1" in markers
    for other in (12, 16, 32):
        if other != width:
            assert "IFS-MODULE:fat" + str(other) + ":" not in markers, (
                identifier, "foreign FAT module metadata"
            )
    dynamic = run("readelf", "-d", binary, capture=True)
    assert "(NEEDED)" not in dynamic, identifier
    return payload


def main():
    with tempfile.TemporaryDirectory(prefix="fat-release-isolation-") as temp:
        work = Path(temp)
        root = work / "target"
        (root / "var/lib/dpkg/info").mkdir(parents=True)
        (root / "var/lib/dpkg/updates").mkdir(parents=True)
        (root / "var/lib/dpkg/status").touch()
        archives = {}
        inventories = []
        try:
            for item in RELEASES:
                width, version, expected_sha, _ = item
                filename = package(item) + "_" + version + "_amd64.deb"
                archive = work / filename
                url = ("https://github.com/Infiltrator-Projects/Filesystem-FAT" +
                       str(width) + "/releases/download/v" + version +
                       "/" + filename)
                run("curl", "-fL", "--retry", "3", "-sS", "-o", archive, url)
                assert hashlib.sha256(archive.read_bytes()).hexdigest() == expected_sha
                archives[slug(item)] = archive
                inventories.append(inspect(archive, item, work))

            # Format-specific modules and manuals must not collide.
            for i in range(len(inventories)):
                for j in range(i + 1, len(inventories)):
                    assert inventories[i].isdisjoint(inventories[j]), (
                        "FAT package payload overlap", i, j
                    )

            def dpkg(*args):
                run("sudo", "dpkg", "--no-triggers", "--root=" + str(root),
                    "--force-depends", *args)

            for item in RELEASES:
                dpkg("--install", archives[slug(item)])
            for item in RELEASES:
                assert status(root, package(item)) == "install ok installed"
                assert (root / "System/Filesystems" / so(item)).is_file()

            # Remove each independent FAT module, ensure other two remain,
            # reinstall, and continue to the next format.
            for item in RELEASES:
                dpkg("--remove", package(item))
                assert status(root, package(item)) != "install ok installed"
                assert not (root / "System/Filesystems" / so(item)).exists()
                for other in RELEASES:
                    if other != item:
                        assert status(root, package(other)) == "install ok installed"
                        assert (root / "System/Filesystems" / so(other)).is_file()
                dpkg("--install", archives[slug(item)])
                assert status(root, package(item)) == "install ok installed"
                assert (root / "System/Filesystems" / so(item)).is_file()
            print("FAT12-FAT16-FAT32-INDEPENDENT-PACKAGE-ISOLATION-PASS")
        finally:
            # Private root contains root-owned dpkg metadata.
            run("sudo", "chown", "-R",
                str(os.getuid()) + ":" + str(os.getgid()), root)


if __name__ == "__main__":
    main()

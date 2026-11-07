#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Exercise published FFS/exFAT package coexistence in an isolated dpkg root.

Release binaries are downloaded and verified against their published SHA256
digests. All install/remove operations target only the private temporary root
and private dpkg database on the disposable CI runner.
"""
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile

RELEASES = (
    ("FFS", "ffs", "1.0.7",
     "7a358bb9d690fb8fc44dc4a9c2388f624225252915c32a6458e8442a8bd16200",
     "dc62ac594c6959b6fd03093a30fb9c6e0fdc9b1c",
     "infiltrator_filesystem_ffs_defrag_v1"),
    ("exFAT", "exfat", "0.1.10",
     "670f97d0a1c14e9f3fdf721b9dcf2c20182cff90525b7169ba0e349bba1a7df5",
     "02d054202e4bbb2b3f1da36391019b64975842cb",
     "infiltrator_filesystem_exfat_defrag_v1"),
)


def run(*command, capture=False):
    result = subprocess.run([str(arg) for arg in command], text=True,
                            check=True, capture_output=capture)
    return result.stdout.strip() if capture else ""


def package_name(item):
    return "infiltrator-filesystem-" + item[1]


def module_name(item):
    return item[0] + ".so"


def package_field(archive, name):
    return run("dpkg-deb", "-f", archive, name, capture=True)


def inventory(root):
    assert not any(path.is_symlink() for path in root.rglob("*"))
    return set(str(p.relative_to(root)) for p in root.rglob("*") if p.is_file())



def verify_format_binary(binary, item, expected_sha256):
    """Apply the same read-only checks to genuine and hostile local fixtures."""
    if not binary.is_file() or binary.is_symlink():
        return False
    if hashlib.sha256(binary.read_bytes()).hexdigest() != expected_sha256:
        return False
    try:
        hdr = run("readelf", "-h", binary, capture=True)
        exports = run("readelf", "--dyn-syms", "-W", binary, capture=True)
    except (subprocess.CalledProcessError, OSError):
        return False
    symbol = item[5]
    other = "exfat" if item[1] == "ffs" else "ffs"
    return ("ELF64" in hdr and "DYN (" in hdr and
            "Advanced Micro Devices X86-64" in hdr and
            symbol in exports and
            ("infiltrator_filesystem_" + other + "_defrag_v1") not in exports)


def reject_bad_module_fixtures(work, item, archive, other_item, other_archive):
    """Damage only an extracted disposable SO; never touch /System on the host."""
    module = work / (item[1] + "-unpacked/System/Filesystems") / module_name(item)
    other = (work / (other_item[1] + "-unpacked/System/Filesystems") /
             module_name(other_item))
    assert module.is_file() and other.is_file()
    original = module.read_bytes()
    digest = package_field(archive, "X-Infiltrator-Resource-SHA256")
    foreign_digest = package_field(other_archive, "X-Infiltrator-Resource-SHA256")
    assert verify_format_binary(module, item, digest)
    try:
        module.write_bytes(original + b"\x00")
        assert not verify_format_binary(module, item, digest), "tamper accepted"
        module.unlink()
        assert not verify_format_binary(module, item, digest), "missing SO accepted"
        module.write_bytes(b"not an ELF shared object")
        assert not verify_format_binary(module, item, digest), "text accepted"
        module.write_bytes(other.read_bytes())
        assert not verify_format_binary(module, item, foreign_digest), (
            "foreign format accepted with matching foreign digest"
        )
    finally:
        module.write_bytes(original)
    assert verify_format_binary(module, item, digest), "restored SO rejected"

def validate(archive, item, staging):
    _, slug, _, _, revision, symbol = item
    assert package_field(archive, "Package") == package_name(item)
    assert package_field(archive, "X-Infiltrator-Source-Revision") == revision
    control = staging.parent / (slug + "-control")
    run("dpkg-deb", "-e", archive, control)
    for forbidden in ("preinst", "postinst", "prerm", "postrm", "triggers"):
        assert not (control / forbidden).exists(), "unexpected maintainer script"
    run("dpkg-deb", "-x", archive, staging)
    expected_module = "System/Filesystems/" + module_name(item)
    expected_pdf = "System/Resources/Documentation/Filesystem-" + item[0] + ".pdf"
    files = inventory(staging)
    assert expected_module in files and expected_pdf in files, files
    expected_kernel = "System/Kernel/Filesystems/" + slug + ".ko"
    assert files in ({expected_module, expected_pdf},
                     {expected_module, expected_pdf, expected_kernel}), files
    binary = staging / expected_module
    pdf = staging / expected_pdf
    assert pdf.read_bytes().startswith(b"%PDF-")
    assert verify_format_binary(
        binary, item, package_field(archive, "X-Infiltrator-Resource-SHA256")
    ), "FFS/exFAT payload identity, ELF or digest invalid"
    assert hashlib.sha256(pdf.read_bytes()).hexdigest() == package_field(
        archive, "X-Infiltrator-PDF-SHA256"
    )

    return files


def installed(root, item):
    status_file = root / "var/lib/dpkg/status"
    return ("Package: " + package_name(item) + "\n") in status_file.read_text() and (
        "Status: install ok installed\n" in status_file.read_text()
    )


def main():
    with tempfile.TemporaryDirectory(prefix="independent-fs-qa-") as directory:
        work = Path(directory)
        root = work / "target"
        (root / "var/lib/dpkg/info").mkdir(parents=True)
        (root / "var/lib/dpkg/updates").mkdir(parents=True)
        (root / "var/lib/dpkg/status").touch()
        archives = {}
        payloads = []
        try:
            for item in RELEASES:
                upstream, slug, version, sha, _, _ = item
                filename = package_name(item) + "_" + version + "_amd64.deb"
                archive = work / filename
                url = ("https://github.com/Infiltrator-Projects/Filesystem-" +
                       upstream + "/releases/download/v" + version + "/" + filename)
                run("curl", "-fL", "--retry", "3", "-sS", "-o", archive, url)
                assert hashlib.sha256(archive.read_bytes()).hexdigest() == sha
                archives[slug] = archive
                payloads.append(validate(archive, item, work / (slug + "-unpacked")))
            assert payloads[0].isdisjoint(payloads[1]), "overlapping payload paths"

            for current in RELEASES:
                foreign = next(item for item in RELEASES if item != current)
                reject_bad_module_fixtures(
                    work, current, archives[current[1]],
                    foreign, archives[foreign[1]]
                )
            print("FFS-EXFAT-NEGATIVE-PACKAGE-IDENTITY-PASS")

            def dpkg(*args):
                run("sudo", "dpkg", "--no-triggers", "--root=" + str(root),
                    "--force-depends", *args)

            for item in RELEASES:
                dpkg("--install", archives[item[1]])
            for item in RELEASES:
                assert installed(root, item)
                assert (root / "System/Filesystems" / module_name(item)).is_file()
            for item in RELEASES:
                other = next(it for it in RELEASES if it != item)
                dpkg("--remove", package_name(item))
                assert not (root / "System/Filesystems" / module_name(item)).exists()
                assert (root / "System/Filesystems" / module_name(other)).is_file()
                assert installed(root, other)
                dpkg("--install", archives[item[1]])
                assert installed(root, item)
            print("INDEPENDENT-FFS-EXFAT-ISOLATED-DPKG-COEXISTENCE-PASS")
        finally:
            # sudo dpkg writes root-owned status/files under private target.
            run("sudo", "chown", "-R", str(os.getuid()) + ":" + str(os.getgid()), root)


if __name__ == "__main__":
    main()

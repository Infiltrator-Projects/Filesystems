#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Exercise the real guard through APT with a private dpkg database.

The dpkg executor is an inert recorder which always refuses mutation. Neither
APT nor dpkg can reach the host status database, package lists or archives.
"""

import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

guard = Path(sys.argv[1]).resolve()

with tempfile.TemporaryDirectory(prefix="filesystems-removal-") as temporary:
    root = Path(temporary)
    for directory in ("dpkg/updates", "lists/partial", "archives/partial",
                      "apt.conf.d", "sources.list.d"):
        (root / directory).mkdir(parents=True)
    status = root / "dpkg/status"
    marker = root / "dpkg-called"
    executor = root / "dpkg-recorder"
    executor.write_text(
        "#!/usr/bin/python3\n"
        "import pathlib, sys\n"
        "if '--version' in sys.argv:\n"
        "    print('Debian dpkg version 1.22.0 (amd64).')\n"
        "    raise SystemExit(0)\n"
        "if '--print-foreign-architectures' in sys.argv:\n"
        "    raise SystemExit(0)\n"
        "if '--print-architecture' in sys.argv:\n"
        "    print('amd64')\n"
        "    raise SystemExit(0)\n"
        "if any(a.startswith('--assert-') for a in sys.argv[1:]):\n"
        "    raise SystemExit(0)\n"
        "if '--set-selections' in sys.argv:\n"
        "    sys.stdin.read()\n"
        "    raise SystemExit(0)\n"
        f"pathlib.Path({str(marker)!r}).write_text(' '.join(sys.argv[1:]))\n"
        "raise SystemExit(42)\n",
        encoding="utf-8")
    executor.chmod(0o755)

    environment = os.environ.copy()
    environment.update(LC_ALL="C", DPKG_ADMINDIR=str(status.parent))
    # APT reads this before its normal configuration directories. Setting
    # Dir::Etc only on argv is too late to exclude host-defined dpkg hooks.
    configuration = root / "apt.conf"
    configuration.write_text(
        f'Dir::Etc::parts "{root / "apt.conf.d"}";\n'
        'Dir::Etc::main "/dev/null";\n', encoding="utf-8")
    environment["APT_CONFIG"] = str(configuration)

    def installed(name, extra="", architecture="amd64"):
        return (f"Package: {name}\nStatus: install ok installed\n"
                f"Architecture: {architecture}\nVersion: 1\n"
                f"Description: disposable transaction fixture\n{extra}\n")

    def invoke(approved, database):
        status.write_text(database, encoding="utf-8")
        marker.unlink(missing_ok=True)
        command = ["/usr/bin/apt-get"]
        options = {
            "Dir::State::status": str(status),
            "Dir::State::extended_states": str(root / "extended_states"),
            "Dir::State::lists": str(root / "lists"),
            "Dir::Cache::archives": str(root / "archives"),
            "Dir::Cache::pkgcache": "",
            "Dir::Cache::srcpkgcache": "",
            "Dir::Etc::main": "/dev/null",
            "Dir::Etc::parts": str(root / "apt.conf.d"),
            "Dir::Etc::sourcelist": "/dev/null",
            "Dir::Etc::sourceparts": str(root / "sources.list.d"),
            "Dir::Bin::dpkg": str(executor),
            "Dir::Log": str(root),
            "APT::Architecture": "amd64",
            # A private fixture needs no download user; some isolated test
            # containers cannot switch to the host's _apt numeric identity.
            "APT::Sandbox::User": "root",
            "Debug::NoLocking": "0",
            "DPkg::ConfigurePending": "0",
            "APT::Get::Purge": "0",
            "DPkg::Pre-Install-Pkgs::": shlex.join([str(guard), *approved]),
            f"DPkg::Tools::Options::{guard}::Version": "3",
            f"DPkg::Tools::Options::{guard}::InfoFD": "0",
        }
        for key, value in options.items():
            command.extend(["-o", f"{key}={value}"])
        command.extend(["remove", "-y", "ntfs-3g"])
        return subprocess.run(command, env=environment, text=True,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                              timeout=30)

    approved = ["ntfs-3g:amd64"]
    ordinary = installed("ntfs-3g", "Priority: optional\n")
    result = invoke(approved, ordinary)
    assert marker.exists() and "--remove" in marker.read_text(), (
        result.stdout, marker.read_text() if marker.exists() else "no dpkg call")

    # Both names are catalogue-managed. The newly dependent second package
    # must still be refused because it was absent from the user's approval.
    changed = ordinary + installed("hfsprogs", "Depends: ntfs-3g\n")
    result = invoke(approved, changed)
    assert result.returncode != 0 and not marker.exists(), result.stdout
    assert "plan changed after confirmation" in result.stdout, result.stdout

    result = invoke(["ntfs-3g:amd64", "hfsprogs:amd64"], changed)
    assert marker.exists() and "--remove" in marker.read_text(), result.stdout

    result = invoke(approved, installed("ntfs-3g", "Priority: required\n"))
    assert result.returncode != 0 and not marker.exists(), result.stdout
    assert "marked Essential, Protected, or required" in result.stdout, result.stdout

    result = invoke(["ntfs-3g:i386"], ordinary)
    assert result.returncode != 0 and not marker.exists(), result.stdout

    result = invoke(["ntfs-3g:all"], installed("ntfs-3g", architecture="all"))
    assert marker.exists() and "--remove" in marker.read_text(), result.stdout

    # Bypassing APT does not make the helper a general-purpose approval oracle.
    environment.pop("DPKG_FRONTEND_LOCKED", None)
    result = subprocess.run([str(guard), *approved], env=environment,
                            input="VERSION 3\n\n", text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            timeout=10)
    assert result.returncode != 0 and "locked transaction" in result.stdout, result.stdout

print("APT rejects changed approvals before issuing any dpkg removal command.")

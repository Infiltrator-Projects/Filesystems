#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Fail closed if extracted formats regain central compiler/removal helpers."""
import pathlib
import sys

root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
active = [root / "src" / name for name in (
    "main.cpp", "probe.cpp", "probe.hpp", "installer.cpp", "installer.hpp",
    "desktop_actions.cpp", "desktop_actions.hpp",
    "action_policy.cpp", "action_policy.hpp")]
for path in active:
    source = path.read_text()
    for fmt in ("fat12", "fat16", "fat32", "ofs", "ext2", "minix"):
        assert fmt + "-module-helper" not in source, (str(path), fmt)
    for obsolete in (
        "manage_fat16_module_async", "manage_fat12_module_async",
        "manage_fat32_module_async", "manage_ofs_module_async",
        "manage_ext2_dropin_async", "manage_minix_dropin_async",
        "InstallDropIn", "RemoveDropIn", "fat12_button", "fat32_button",
        "ext2_dropin_button", "minix_dropin_button",
    ):
        assert obsolete not in source, (str(path), obsolete)

policy = (root / "src/action_policy.cpp").read_text()
assert 'policy.button.label = "Independent package";' in policy
assert "descriptor.provider == SupportProvider::Independent" in policy
assert not (root / "native/providers").exists()
print("RETIRED-HELPER-BOUNDARY-PASS")

#!/usr/bin/env python3

# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0

"""
POSIX module wrapper around Zephyr's check_compliance.py.

Loads repo-specific and optional local Kconfig allowlists before running the
standard compliance checks.
"""

import os
import re
import subprocess
import sys
from pathlib import Path

import yaml

SCRIPT_DIR = Path(__file__).resolve().parent
ALLOWLIST_BASENAMES = (
    "undef_kconfig_allowlist.yaml",
    "undef_kconfig_allowlist.local.yaml",
)


def find_zephyr_base():
    zephyr_base = os.environ.get("ZEPHYR_BASE")
    if zephyr_base:
        return Path(zephyr_base)

    try:
        cp = subprocess.run(
            ("west", "list", "-f", "{abspath}", "zephyr"),
            capture_output=True,
            text=True,
            check=True,
        )
    except (OSError, subprocess.CalledProcessError) as exc:
        sys.exit(
            f"{__file__}: error: ZEPHYR_BASE is unset and 'west list zephyr' failed: {exc}"
        )

    zephyr_base = cp.stdout.strip()
    if not zephyr_base:
        sys.exit(f"{__file__}: error: could not resolve Zephyr base path")

    os.environ["ZEPHYR_BASE"] = zephyr_base
    return Path(zephyr_base)


def load_allowlist(path):
    with open(path, encoding="utf-8") as fp:
        data = yaml.safe_load(fp)

    if data is None:
        return set(), set()

    if isinstance(data, list):
        return set(data), set()

    outside = set(data.get("outside") or [])
    within = set(data.get("within") or [])
    return outside, within


def load_allowlists():
    outside = set()
    within = set()

    for basename in ALLOWLIST_BASENAMES:
        path = SCRIPT_DIR / basename
        if not path.is_file():
            continue
        file_outside, file_within = load_allowlist(path)
        outside |= file_outside
        within |= file_within

    return outside, within


def extend_kconfig_allowlists(check_compliance, outside):
    if not outside:
        return

    for cls in check_compliance.inheritors(check_compliance.ComplianceTest):
        if not hasattr(cls, "UNDEF_KCONFIG_ALLOWLIST"):
            continue
        if cls.CONFIG_ != "CONFIG_":
            continue
        cls.UNDEF_KCONFIG_ALLOWLIST = cls.UNDEF_KCONFIG_ALLOWLIST | outside


def patch_within_kconfig_allowlist(check_compliance, within):
    if not within:
        return

    allowlist_hint = ", ".join(ALLOWLIST_BASENAMES)

    def check_no_undef_within_kconfig(self, kconf):
        warnings = []
        for warning in kconf.warnings:
            if "undefined symbol" not in warning:
                continue
            match = re.search(r"undefined symbol (\w+):", warning)
            if match and match.group(1) in within:
                continue
            warnings.append(warning)

        undef_ref_warnings = "\n\n\n".join(warnings)
        if undef_ref_warnings:
            self.failure(
                f"Undefined Kconfig symbols:\n\n {undef_ref_warnings}\n\n"
                f"If any of these are false positives, add them to 'within:' in "
                f"{allowlist_hint}."
            )

    check_compliance.KconfigCheck.check_no_undef_within_kconfig = (
        check_no_undef_within_kconfig
    )


def main(argv=None):
    find_zephyr_base()
    outside, within = load_allowlists()

    zephyr_ci = Path(os.environ["ZEPHYR_BASE"]) / "scripts" / "ci"
    sys.path.insert(0, str(zephyr_ci))

    import check_compliance

    extend_kconfig_allowlists(check_compliance, outside)
    patch_within_kconfig_allowlist(check_compliance, within)

    check_compliance.main(argv)


if __name__ == "__main__":
    main(sys.argv[1:])

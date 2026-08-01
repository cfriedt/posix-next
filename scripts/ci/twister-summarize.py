#!/usr/bin/env python3
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0
"""Summarize twister.json results into an abridged, checked-in JSON snapshot.

Reads one or more (possibly sharded) twister.json files and produces a small,
stable-diff summary of POSIX test suite results, grouped by option-group token
and twister scenario variant. The output is committed to doc/metrics/ and
consumed at doc-build time by scripts/doc/posix_metrics.py to render badges.

Suite names are expected to look like:

    portability.posix.<group>[.<variant>]

where <group> never contains dots and <variant> may be multi-token
(e.g. "minimal.strsignal_no_desc"). Variants are recorded generically; the
doc-build layer decides which variants (linux_compat, ubsan, asan,
static_analysis, ...) map to badges, so new scenario classes require no
changes here.

Group tokens are recorded verbatim (no alias resolution) so the checked-in
snapshot never needs regenerating when doc pages move; alias resolution
happens in scripts/doc/posix_metrics.py.

Twister nightly runs use --retry-failed; the final statuses in twister.json
already reflect retries. When the same (name, platform) instance appears in
multiple input files (e.g. twister-out.N retry directories), the last file
listed wins.

Sanitizer runs (asan / ubsan CI profiles) reuse the regular suites with
extra Kconfig, so their twister.json carries no distinguishing variant
token. Pass ``--force-variant asan`` to collapse every suite of the run
into that single scenario class, and ``--findings`` (the output of
sanitizer-findings.py) to record per-scenario ``failed_functions`` — the
implementation functions implicated by sanitizer reports. When --findings
is given, every scenario carries a failed_functions list (possibly empty:
attribution ran and implicated nothing); without it the key is absent and
consumers fall back to suite-level status.
"""

import argparse
import datetime
import json
import sys
from pathlib import Path

SUITE_PREFIX = "portability.posix."

# Instance statuses that make a (group, variant) scenario "failed".
FAIL_STATUSES = frozenset({"failed", "error", "blocked"})

# Statuses tallied in per-scenario counts, in output order.
COUNTED_STATUSES = (
    "passed",
    "failed",
    "error",
    "blocked",
    "skipped",
    "filtered",
    "notrun",
)

SCHEMA_VERSION = 1


def parse_args(argv):
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        allow_abbrev=False,
    )
    parser.add_argument("--output", required=True, type=Path, help="output JSON path")
    parser.add_argument(
        "--commit", default="", help="commit sha the results were built from"
    )
    parser.add_argument("--run-url", default="", help="workflow run URL")
    parser.add_argument("--profile", default="", help="CI config profile name")
    parser.add_argument(
        "--shards-expected", type=int, default=None, help="number of shards planned"
    )
    parser.add_argument(
        "--prefix",
        default=SUITE_PREFIX,
        help=f"suite name prefix filter (default {SUITE_PREFIX})",
    )
    parser.add_argument(
        "--created-at",
        default=None,
        help="ISO 8601 timestamp for provenance (default: now, UTC)",
    )
    parser.add_argument(
        "--force-variant",
        default=None,
        metavar="NAME",
        help="record every suite under this single scenario variant "
        "(for sanitizer runs whose suite names carry no variant token)",
    )
    parser.add_argument(
        "--findings",
        type=Path,
        default=None,
        help="sanitizer-findings.py JSON; adds per-scenario failed_functions",
    )
    parser.add_argument("inputs", nargs="+", type=Path, help="twister.json files")
    return parser.parse_args(argv)


def load_instances(paths, prefix):
    """Yield (shards_reported, {(name, platform): status}) merged last-wins."""
    instances = {}
    shards_reported = 0
    for path in paths:
        try:
            with path.open() as f:
                data = json.load(f)
            suites = data["testsuites"]
        except (OSError, ValueError, KeyError) as e:
            print(f"warning: skipping {path}: {e}", file=sys.stderr)
            continue
        shards_reported += 1
        for suite in suites:
            name = suite.get("name", "")
            if not name.startswith(prefix):
                continue
            platform = suite.get("platform", "")
            status = suite.get("status", "")
            instances[(name, platform)] = status
    return shards_reported, instances


def split_suite_name(name, prefix):
    rest = name[len(prefix) :]
    parts = rest.split(".")
    return parts[0], ".".join(parts[1:]) or "base"


def load_findings(path, prefix):
    """suite name -> [implicated functions] from sanitizer-findings.py output."""
    if path is None:
        return None
    try:
        with path.open() as f:
            data = json.load(f)
    except (OSError, ValueError) as e:
        print(f"warning: ignoring findings {path}: {e}", file=sys.stderr)
        return None
    return {
        name: sorted(entry.get("functions") or [])
        for name, entry in (data.get("suites") or {}).items()
        if name.startswith(prefix)
    }


def summarize(instances, prefix, force_variant=None, findings=None):
    groups = {}
    for (name, platform), status in instances.items():
        group, variant = split_suite_name(name, prefix)
        if force_variant:
            variant = force_variant
        scenario = groups.setdefault(group, {"scenarios": {}})["scenarios"].setdefault(
            variant,
            {
                "counts": {s: 0 for s in COUNTED_STATUSES},
                "platforms": set(),
                "failed_platforms": set(),
            },
        )
        if status in scenario["counts"]:
            scenario["counts"][status] += 1
        scenario["platforms"].add(platform)
        if status in FAIL_STATUSES:
            scenario["failed_platforms"].add(platform)

    implicated = {}
    if findings is not None:
        for name, functions in findings.items():
            group, variant = split_suite_name(name, prefix)
            if force_variant:
                variant = force_variant
            implicated.setdefault((group, variant), set()).update(functions)

    for key, group in groups.items():
        for variant, scenario in group["scenarios"].items():
            counts = scenario["counts"]
            if any(counts[s] for s in FAIL_STATUSES):
                status = "failed"
            elif counts["passed"]:
                status = "passed"
            else:
                status = "skipped"
            scenario["status"] = status
            scenario["platforms"] = sorted(scenario["platforms"])
            scenario["failed_platforms"] = sorted(scenario["failed_platforms"])
            # drop always-zero buckets to keep the checked-in file small
            scenario["counts"] = {k: v for k, v in counts.items() if v}
            if findings is not None:
                scenario["failed_functions"] = sorted(
                    implicated.get((key, variant), ())
                )
    return groups


def main(argv=None):
    args = parse_args(argv)

    shards_reported, instances = load_instances(args.inputs, args.prefix)
    if shards_reported == 0:
        print("error: no readable twister.json inputs", file=sys.stderr)
        return 1
    findings = load_findings(args.findings, args.prefix)

    created_at = args.created_at or (
        datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    )
    summary = {
        "schema_version": SCHEMA_VERSION,
        "provenance": {
            "commit": args.commit,
            "commit_short": args.commit[:7],
            "created_at": created_at,
            "profile": args.profile,
            "shards_expected": args.shards_expected,
            "shards_reported": shards_reported,
            "workflow_run_url": args.run_url,
        },
        "groups": summarize(instances, args.prefix, args.force_variant, findings),
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w") as f:
        json.dump(summary, f, indent=2, sort_keys=True)
        f.write("\n")

    ngroups = len(summary["groups"])
    nscen = sum(len(g["scenarios"]) for g in summary["groups"].values())
    print(
        f"{args.output}: {ngroups} groups, {nscen} scenarios, {shards_reported} shard(s)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

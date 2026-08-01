#!/usr/bin/env python3
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0
"""Extract sanitizer-implicated POSIX functions from twister output logs.

Walks twister output directories for per-instance ``handler.log`` files and
parses AddressSanitizer / LeakSanitizer / UndefinedBehaviorSanitizer reports
out of the captured console output. Stack frames (and UBSAN error sites)
inside the POSIX module implementation (``modules/lib/posix/lib/posix/**``)
are attributed to their enclosing function; the result maps twister suite
names to the implicated function names.

Sanitizers only report what fails: a function that never appears in any
report is presumed clean. The output feeds ``twister-summarize.py
--findings``, which records per-scenario ``failed_functions`` in the
checked-in summary; the doc layer then renders per-function badges green
unless implicated (and falls back to the suite status when a failure could
not be attributed to any implementation function).

Run twister with ``--enable-asan`` / ``--enable-ubsan``. For function
attribution of UBSAN errors, export ``UBSAN_OPTIONS=print_stacktrace=1``
before running twister (its handler appends its own ``log_path`` /
``halt_on_error`` settings to the inherited value).
"""

import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "doc"))
import posix_metrics  # noqa: E402  (function_spans)

IMPL_MARKER = "modules/lib/posix/lib/posix/"

# A sanitizer report starts...
_REPORT_START_RE = re.compile(
    r"==\d+==\s*ERROR: (?:Address|Leak|Memory|Thread)Sanitizer"
    r"|: runtime error: "
    r"|==\d+==\s*WARNING: \w+Sanitizer"
)
# ... and ends at its summary (or abort) line.
_REPORT_END_RE = re.compile(r"SUMMARY: \w+Sanitizer|==\d+==\s*ABORTING")

# "    #3 0x7f... in mq_notify /path/to/mqueue.c:123:5"
_FRAME_RE = re.compile(
    r"^\s*#\d+ 0x[0-9a-fA-F]+ in ([A-Za-z_]\w*)(?:\(.*?\))? ([^\s]+?):(\d+)"
)
# "/path/to/timer.c:88:10: runtime error: signed integer overflow ..."
_UBSAN_SITE_RE = re.compile(r"^([^\s:]+):(\d+):\d+: runtime error: ")


def _impl_path(path: str) -> bool:
    return IMPL_MARKER in path.replace("\\", "/")


class _SpanCache:
    """file path -> sorted (name, first, last) function spans."""

    def __init__(self):
        self._cache = {}

    def function_at(self, path: str, line: int) -> str | None:
        spans = self._cache.get(path)
        if spans is None:
            spans = posix_metrics.function_spans(Path(path))
            self._cache[path] = spans
        for name, first, last in spans:
            if first <= line <= last:
                return name
        return None


def parse_log(text: str, spans: _SpanCache) -> tuple[set[str], int]:
    """Return (implicated implementation functions, report count)."""
    functions: set[str] = set()
    reports = 0
    active = False
    for line in text.splitlines():
        site = _UBSAN_SITE_RE.match(line)
        if site:
            reports += 1
            active = True
            if _impl_path(site.group(1)):
                fn = spans.function_at(site.group(1), int(site.group(2)))
                if fn:
                    functions.add(fn)
            continue
        if _REPORT_START_RE.search(line):
            reports += 1
            active = True
            continue
        if _REPORT_END_RE.search(line):
            active = False
            continue
        if not active:
            continue
        frame = _FRAME_RE.match(line)
        if frame and _impl_path(frame.group(2)):
            functions.add(frame.group(1))
    return functions, reports


def collect(outdirs, prefix: str) -> dict:
    """suite name -> {"functions": [...], "reports": n} across all outdirs."""
    spans = _SpanCache()
    suites: dict[str, dict] = {}
    logs = 0
    for outdir in outdirs:
        for log in sorted(Path(outdir).rglob("handler.log")):
            suite = log.parent.name
            if not suite.startswith(prefix):
                continue
            try:
                text = log.read_text(errors="replace")
            except OSError as e:
                print(f"warning: skipping {log}: {e}", file=sys.stderr)
                continue
            logs += 1
            functions, reports = parse_log(text, spans)
            if not reports:
                continue
            entry = suites.setdefault(suite, {"functions": set(), "reports": 0})
            entry["functions"] |= functions
            entry["reports"] += reports
    for entry in suites.values():
        entry["functions"] = sorted(entry["functions"])
    print(
        f"scanned {logs} handler.log(s): {len(suites)} suite(s) with sanitizer reports"
    )
    return suites


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        allow_abbrev=False,
    )
    parser.add_argument("--output", required=True, type=Path, help="output JSON path")
    parser.add_argument(
        "--prefix",
        default="portability.posix.",
        help="suite name prefix filter (default portability.posix.)",
    )
    parser.add_argument(
        "outdirs", nargs="+", type=Path, help="twister output directories"
    )
    args = parser.parse_args(argv)

    outdirs = [d for d in args.outdirs if d.is_dir()]
    if not outdirs:
        print("error: no twister output directories found", file=sys.stderr)
        return 1

    summary = {"schema_version": 1, "suites": collect(outdirs, args.prefix)}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w") as f:
        json.dump(summary, f, indent=2, sort_keys=True)
        f.write("\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())

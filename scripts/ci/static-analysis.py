#!/usr/bin/env python3
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0
"""Run the clang static analyzer over POSIX module sources built by twister.

Collects ``compile_commands.json`` from twister build directories (a
``--build-only`` run of the ``static_analysis`` CI profile), filters the
compile commands down to the POSIX module implementation
(``modules/lib/posix/lib/posix/**``) plus the Zephyr-tree sources this
module introduces via its patch series, and analyzes the result with
scan-build's ``analyze-build`` driver. Its plist reports are condensed into
a small findings JSON snapshot for doc/metrics/.

Kconfig variants matter: a file compiled under different configurations
takes different preprocessor branches. Instead of analyzing one arbitrary
build per file, commands are deduplicated by (source file, preprocessed
content hash), so every distinct configuration variant of a file that any
test build produces is analyzed.

The analyzer runs with cross-translation-unit analysis (when
clang-extdef-mapping is available), Z3 refutation of findings (when clang
was built with Z3), header analysis, and the ``optin.portability.UnixAPI``
and ``security.FloatLoopCounter`` checkers on top of the defaults.

Like the sanitizers, the analyzer only reports what it can prove wrong: a
function absent from the findings is presumed clean, so the doc layer
renders per-function badges green unless a finding lands in that function.

Output shape::

    {
      "schema_version": 1,
      "provenance": {..., "analysis": {"ctu": bool, "z3": bool, ...}},
      "analyzed_files": ["lib/posix/mqueue/mqueue.c", ...],
      "findings": [
        {"file": "lib/posix/...", "line": 42, "function": "mq_notify",
         "checker": "core.NullDereference", "description": "..."}
      ]
    }

File paths are module-root relative (``lib/posix/...``, ``include/...``)
or ``zephyr/``-prefixed for patched Zephyr-tree sources, so the snapshot
is stable across checkouts.
"""

import argparse
import concurrent.futures
import datetime
import hashlib
import json
import os
import plistlib
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "doc"))
import posix_metrics  # noqa: E402  (function_spans)

MODULE_ROOT = Path(__file__).resolve().parents[2]

IMPL_MARKER = "modules/lib/posix/lib/posix/"
HDR_MARKER = "modules/lib/posix/include/"

# a runaway generator aside, no file legitimately has this many Kconfig
# variants across the test builds; drop the excess loudly rather than
# ballooning the analysis
MAX_VARIANTS_PER_FILE = 8

_ANALYZE_BUILD_NAMES = ("analyze-build",) + tuple(
    f"analyze-build-{v}" for v in range(22, 13, -1)
)
_EXTDEF_MAPPING_NAMES = ("clang-extdef-mapping",) + tuple(
    f"clang-extdef-mapping-{v}" for v in range(22, 13, -1)
)

_EXTRA_CHECKERS = ("optin.portability.UnixAPI", "security.FloatLoopCounter")


def find_tool(explicit: str | None, names, what: str) -> str:
    if explicit:
        return explicit
    for name in names:
        path = shutil.which(name)
        if path:
            return path
    print(f"error: cannot find {what} (tried {', '.join(names)})", file=sys.stderr)
    raise SystemExit(1)


def find_clang() -> str | None:
    """A real clang binary — never a ccache shim, which confuses --analyze."""
    names = ("clang",) + tuple(f"clang-{v}" for v in range(22, 13, -1))
    for name in names:
        for prefix in ("/usr/bin", "/usr/local/bin"):
            path = Path(prefix) / name
            if path.is_file():
                return str(path)
    path = shutil.which("clang")
    if path and "ccache" not in path:
        return path
    return None


def z3_supported(clang: str) -> bool:
    """Whether this clang was built with the Z3 constraint solver."""
    probe = subprocess.run(
        [
            clang,
            "--analyze",
            "-Xclang",
            "-analyzer-config",
            "-Xclang",
            "crosscheck-with-z3=true",
            "-x",
            "c",
            "-c",
            os.devnull,
            "-o",
            os.devnull,
        ],
        capture_output=True,
    )
    return probe.returncode == 0


def patched_zephyr_sources(module_root: Path) -> frozenset:
    """Zephyr-tree C sources this module introduces or modifies via its
    patch series (``+++ b/<path>`` targets in zephyr/patches/zephyr/).

    Test and sample sources are excluded: like the module's own tests,
    they are not implementation and should not gate the badge.
    """
    paths = set()
    for patch in sorted(
        (module_root / "zephyr" / "patches" / "zephyr").glob("*.patch")
    ):
        try:
            text = patch.read_text(errors="replace")
        except OSError:
            continue
        for line in text.splitlines():
            if line.startswith("+++ b/") and line.endswith(".c"):
                path = line[len("+++ b/") :].strip()
                if not path.startswith(("tests/", "samples/")):
                    paths.add(path)
    return frozenset(paths)


def snapshot_rel(path: str, patched: frozenset = frozenset()) -> str | None:
    """Snapshot-stable relative path for a source the badge pipeline owns.

    ``lib/posix/...`` / ``include/...`` for module sources and headers,
    ``zephyr/<path>`` for Zephyr-tree sources from the patch series, None
    for everything else (findings in unpatched Zephyr or toolchain code are
    not this module's signal).
    """
    norm = path.replace("\\", "/")
    if IMPL_MARKER in norm:
        return "lib/posix/" + norm.split(IMPL_MARKER, 1)[1]
    if HDR_MARKER in norm:
        return "include/" + norm.split(HDR_MARKER, 1)[1]
    for p in patched:
        if norm.endswith("/zephyr/" + p):
            return "zephyr/" + p
    return None


def _preprocess_hash(entry: dict) -> str | None:
    """Hash of the preprocessed translation unit: two builds whose Kconfig
    (or other flags) produce identical code collapse to one analysis."""
    if "arguments" in entry:
        cmd = list(entry["arguments"])
    else:
        cmd = shlex.split(entry.get("command", ""))
    out = []
    skip = False
    for arg in cmd:
        if skip:
            skip = False
            continue
        if arg == "-o":
            skip = True
            continue
        if arg == "-c":
            continue
        out.append(arg)
    out += ["-E", "-P", "-o", "-"]
    try:
        proc = subprocess.run(
            out, cwd=entry.get("directory", "."), capture_output=True, timeout=60
        )
    except (OSError, subprocess.TimeoutExpired):
        return None
    if proc.returncode != 0:
        return None
    return hashlib.sha256(proc.stdout).hexdigest()


def collect_compile_commands(outdirs, patched: frozenset):
    """Compile commands for owned sources, deduped by configuration variant.

    Returns (cdb entries, {snapshot-relative path: absolute path}).
    """
    candidates: dict[tuple, dict] = {}
    absolute: dict[str, str] = {}
    ncdb = 0
    for outdir in outdirs:
        for cdb in sorted(Path(outdir).rglob("compile_commands.json")):
            try:
                with cdb.open() as f:
                    commands = json.load(f)
            except (OSError, ValueError) as e:
                print(f"warning: skipping {cdb}: {e}", file=sys.stderr)
                continue
            ncdb += 1
            for entry in commands:
                src = entry.get("file", "")
                if not Path(src).is_absolute():
                    src = str(Path(entry.get("directory", ".")) / src)
                rel = snapshot_rel(src, patched)
                if rel is None:
                    continue
                absolute.setdefault(rel, src)
                candidates[(rel, entry.get("directory", ""))] = entry

    # collapse per-build duplicates down to distinct preprocessed variants
    keys = sorted(candidates)
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        hashes = list(pool.map(lambda k: _preprocess_hash(candidates[k]), keys))
    variants: dict[tuple, dict] = {}
    per_file: dict[str, int] = {}
    dropped = 0
    for key, pph in zip(keys, hashes):
        rel = key[0]
        vkey = (rel, pph if pph is not None else key[1])
        if vkey in variants:
            continue
        if per_file.get(rel, 0) >= MAX_VARIANTS_PER_FILE:
            dropped += 1
            continue
        per_file[rel] = per_file.get(rel, 0) + 1
        variants[vkey] = candidates[key]
    if dropped:
        print(
            f"warning: dropped {dropped} variant(s) beyond "
            f"{MAX_VARIANTS_PER_FILE} per file",
            file=sys.stderr,
        )
    print(
        f"collected {len(absolute)} source(s), {len(variants)} configuration "
        f"variant(s) from {ncdb} build(s)"
    )
    return [variants[k] for k in sorted(variants, key=str)], absolute


def analysis_flags(use_analyzer: str | None) -> tuple[list, dict]:
    """Extra analyze-build flags, plus a provenance record of what ran."""
    flags = ["--analyze-headers"]
    for checker in _EXTRA_CHECKERS:
        flags += ["--enable-checker", checker]
    info = {"ctu": False, "z3": False, "extra_checkers": list(_EXTRA_CHECKERS)}

    extdef = None
    for name in _EXTDEF_MAPPING_NAMES:
        extdef = shutil.which(name)
        if extdef:
            break
    if extdef:
        flags += ["--ctu", "--use-extdef-map-cmd", extdef]
        info["ctu"] = True
    else:
        print(
            "warning: clang-extdef-mapping not found; cross-TU analysis disabled",
            file=sys.stderr,
        )

    if use_analyzer and z3_supported(use_analyzer):
        flags += ["--analyzer-config", "crosscheck-with-z3=true"]
        info["z3"] = True
    else:
        print(
            "warning: clang lacks Z3 support; finding refutation disabled",
            file=sys.stderr,
        )
    return flags, info


def run_analyzer(
    cdb_entries,
    analyze_build: str,
    use_analyzer: str | None,
    extra_flags: list,
    workdir: Path,
) -> Path:
    cdb_path = workdir / "compile_commands.json"
    with cdb_path.open("w") as f:
        json.dump(cdb_entries, f, indent=1)
    report_dir = workdir / "reports"
    report_dir.mkdir()
    # analyze-build parallelizes across translation units on its own
    cmd = [
        analyze_build,
        "--cdb",
        str(cdb_path),
        "-o",
        str(report_dir),
        "--plist-html",
        *extra_flags,
    ]
    if use_analyzer:
        cmd += ["--use-analyzer", use_analyzer]
    print("+", " ".join(cmd))
    proc = subprocess.run(cmd)
    plists = list(report_dir.rglob("*.plist"))
    # analyze-build exits non-zero on driver failure; tolerate it only if it
    # still produced reports (individual translation units can fail to parse)
    if proc.returncode != 0 and not plists:
        print(
            f"error: {analyze_build} exited {proc.returncode} with no reports",
            file=sys.stderr,
        )
        raise SystemExit(1)
    return report_dir


def parse_reports(
    report_dir: Path, absolute: dict[str, str], patched: frozenset
) -> list[dict]:
    spans_cache: dict[str, list] = {}

    def function_at(rel: str, path: str, line: int) -> str | None:
        spans = spans_cache.get(rel)
        if spans is None:
            spans = posix_metrics.function_spans(Path(absolute.get(rel, path)))
            spans_cache[rel] = spans
        for name, first, last in spans:
            if first <= line <= last:
                return name
        return None

    findings = {}
    for plist in sorted(report_dir.rglob("*.plist")):
        try:
            with plist.open("rb") as f:
                data = plistlib.load(f)
        except (OSError, ValueError) as e:
            print(f"warning: skipping {plist}: {e}", file=sys.stderr)
            continue
        files = data.get("files", [])
        for diag in data.get("diagnostics", []):
            loc = diag.get("location", {})
            try:
                path = files[loc.get("file", -1)]
            except IndexError:
                continue
            rel = snapshot_rel(path, patched)
            if rel is None:
                continue
            line = int(loc.get("line", 0))
            function = None
            if diag.get("issue_context_kind") in ("function", "method"):
                function = diag.get("issue_context")
            if not function:
                function = function_at(rel, path, line)
            key = (rel, line, diag.get("check_name", ""))
            findings[key] = {
                "file": rel,
                "line": line,
                "function": function,
                "checker": diag.get("check_name", ""),
                "description": diag.get("description", ""),
            }
    return [findings[k] for k in sorted(findings)]


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
        allow_abbrev=False,
    )
    parser.add_argument("--output", required=True, type=Path, help="output JSON path")
    parser.add_argument(
        "--commit", default="", help="commit sha the sources were built from"
    )
    parser.add_argument("--run-url", default="", help="workflow run URL")
    parser.add_argument(
        "--created-at",
        default=None,
        help="ISO 8601 timestamp for provenance (default: now, UTC)",
    )
    parser.add_argument(
        "--analyze-build", help="path to scan-build's analyze-build driver"
    )
    parser.add_argument("--use-analyzer", help="clang binary for analyze-build to run")
    parser.add_argument(
        "outdirs", nargs="+", type=Path, help="twister output directories"
    )
    args = parser.parse_args(argv)

    outdirs = [d for d in args.outdirs if d.is_dir()]
    if not outdirs:
        print("error: no twister output directories found", file=sys.stderr)
        return 1

    analyze_build = find_tool(args.analyze_build, _ANALYZE_BUILD_NAMES, "analyze-build")
    use_analyzer = args.use_analyzer or find_clang()

    patched = patched_zephyr_sources(MODULE_ROOT)
    cdb_entries, absolute = collect_compile_commands(outdirs, patched)
    if not cdb_entries:
        print("error: no POSIX module compile commands found", file=sys.stderr)
        return 1

    extra_flags, analysis_info = analysis_flags(use_analyzer)
    with tempfile.TemporaryDirectory(prefix="posix-sca-") as tmp:
        report_dir = run_analyzer(
            cdb_entries, analyze_build, use_analyzer, extra_flags, Path(tmp)
        )
        results = parse_reports(report_dir, absolute, patched)

    created_at = args.created_at or (
        datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    )
    version = ""
    if use_analyzer:
        try:
            version = (
                subprocess.run(
                    [use_analyzer, "--version"], capture_output=True, text=True
                )
                .stdout.splitlines()[0]
                .strip()
            )
        except (OSError, IndexError):
            pass
    analysis_info["config_variants"] = len(cdb_entries)
    summary = {
        "schema_version": 1,
        "provenance": {
            "commit": args.commit,
            "commit_short": args.commit[:7],
            "created_at": created_at,
            "tool": "clang scan-build (analyze-build)",
            "analyzer_version": version,
            "analysis": analysis_info,
            "workflow_run_url": args.run_url,
        },
        "analyzed_files": sorted(absolute),
        "findings": results,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w") as f:
        json.dump(summary, f, indent=2, sort_keys=True)
        f.write("\n")
    print(
        f"{args.output}: {len(absolute)} file(s), {len(cdb_entries)} variant(s) "
        f"analyzed, {len(results)} finding(s)"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

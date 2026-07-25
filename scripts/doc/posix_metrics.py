#!/usr/bin/env python3
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0
"""Compute POSIX Option Group metrics for the documentation badges.

This module is the doc-build-time data layer behind the badges rendered by
the posix_badges Sphinx extension. Everything is derived from files already
in the repository:

- Function rosters: the ``:c:func:`` rows of the csv-table in each
  ``doc/posix/option_groups/<name>.rst`` page (the curated, Open Group
  derived source of truth for "which functions belong to this group").
- ENOSYS stubs: static scan of ``lib/posix/**`` sources. A function is a
  stub if and only if its entire body consists of ``ARG_UNUSED(...)``,
  ``errno = ENOSYS``, ``return -1`` and/or ``return ENOSYS`` statements.
- Per-group line coverage: ``doc/metrics/coverage-posix.json`` (gcovr JSON,
  refreshed by the Coverage workflow), attributed to groups by source
  directory via ``doc/posix_group_aliases.yaml``.
- Test scenarios: ``doc/metrics/twister-summary.json`` (refreshed by the
  Twister workflow).
- Curated facts: ``doc/posix_api_overrides.yaml`` (implementation status
  that static analysis cannot see) and ``doc/iso_c_functions.yaml``.

All inputs degrade gracefully: missing or unparseable metrics files yield
``None`` fields rather than errors, so documentation builds never fail for
lack of nightly data (docs CI builds with ``-W``).
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field
from datetime import datetime, timezone
from pathlib import Path

try:
    import yaml
except ImportError:  # pragma: no cover - doc builds always have PyYAML
    yaml = None

TWISTER_SUMMARY_SCHEMA_VERSION = 1

_CFUNC_RE = re.compile(r":c:func:`(\w+)`")
_ROW_RE = re.compile(r"^\s*:c:func:`(\w+)`\s*,(.*)$", re.MULTILINE)
# csv-table rows for non-function symbols (stdin, errno, environ, ...):
# plain identifier followed by the Supported cell
_SYMBOL_ROW_RE = re.compile(r"^[ \t]+([A-Za-z_]\w*)[ \t]*,(.*)$", re.MULTILINE)
_LINE_COMMENT_RE = re.compile(r"//[^\n]*")
_BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
_PREPROC_RE = re.compile(r"^[ \t]*#(?:[^\n\\]|\\\n)*", re.MULTILINE)
_FUNC_NAME_RE = re.compile(r"(\w+)\s*\([^;{}]*\)\s*$", re.DOTALL)
_STUB_STMT_RE = re.compile(
    r"^(?:ARG_UNUSED\s*\(\s*\w+\s*\)|errno\s*=\s*ENOSYS|return\s+-1|return\s+ENOSYS)$"
)
_ENOSYS_STMT_RE = re.compile(r"^(?:errno\s*=\s*ENOSYS|return\s+ENOSYS)$")


@dataclass
class ScenarioSummary:
    status: str  # passed | failed | skipped
    counts: dict[str, int]
    platforms: list[str]
    failed_platforms: list[str]


@dataclass
class GroupMetrics:
    key: str
    functions_total: int = 0
    functions_implemented: int = 0
    stub_functions: list[str] = field(default_factory=list)
    coverage: tuple[int, int] | None = None  # (line_hits, line_total)
    coverage_paths: list[str] = field(default_factory=list)  # repo-relative dirs
    scenarios: dict[str, ScenarioSummary] | None = None

    @property
    def completeness_pct(self) -> float | None:
        if not self.functions_total:
            return None
        return 100.0 * self.functions_implemented / self.functions_total

    @property
    def coverage_pct(self) -> float | None:
        if not self.coverage or not self.coverage[1]:
            return None
        return 100.0 * self.coverage[0] / self.coverage[1]


@dataclass
class PosixMetrics:
    groups: dict[str, GroupMetrics]
    options: dict[str, GroupMetrics]
    option_components: dict[str, list[str]]  # option stem -> component names
    functions: dict[str, dict]  # name -> {"status": str, "coverage": (h, t)|None}
    iso_c_standards: dict[str, str]  # name -> originating standard ("C89", ...)
    iso_c_functions: frozenset[str]
    stub_functions: frozenset[str]
    overrides: dict[str, dict]
    coverage_provenance: dict | None
    twister_provenance: dict | None
    rst_keys: dict[str, str]  # canonical key -> RST page stem

    def rst_key(self, key: str) -> str:
        return self.rst_keys.get(key, key)

    @property
    def twister_age_days(self) -> float | None:
        return _age_days(self.twister_provenance)

    @property
    def coverage_age_days(self) -> float | None:
        return _age_days(self.coverage_provenance)


def _age_days(provenance: dict | None) -> float | None:
    created = (provenance or {}).get("created_at")
    if not created:
        return None
    try:
        then = datetime.fromisoformat(created.replace("Z", "+00:00"))
    except ValueError:
        return None
    return (datetime.now(timezone.utc) - then).total_seconds() / 86400.0


def _load_yaml(path: Path) -> dict:
    if yaml is None or not path.is_file():
        return {}
    try:
        return yaml.safe_load(path.read_text()) or {}
    except yaml.YAMLError:
        return {}


def _load_json(path: Path) -> dict | None:
    try:
        with path.open() as f:
            return json.load(f)
    except (OSError, ValueError):
        return None


def strip_c_comments(text: str) -> str:
    """Remove comments and preprocessor directives from C source text."""
    text = _BLOCK_COMMENT_RE.sub(" ", text)
    text = _LINE_COMMENT_RE.sub(" ", text)
    return _PREPROC_RE.sub(" ", text)


def _iter_functions(text: str):
    """Yield (name, body, name_offset, end_offset) for each top-level
    function definition in C text.

    Brace matching skips string/char literals; nested braces stay inside the
    body. Non-function braces (initializers, struct definitions) are filtered
    by requiring ')' immediately before '{' and a parseable name. Offsets
    index into ``text``: the function name and its closing brace.
    """
    depth = 0
    body_start = None
    sig_end = 0
    sig_start = 0
    in_str = in_chr = esc = False
    for i, ch in enumerate(text):
        if esc:
            esc = False
            continue
        if ch == "\\":
            esc = in_str or in_chr
            continue
        if in_str:
            in_str = ch != '"'
            continue
        if in_chr:
            in_chr = ch != "'"
            continue
        if ch == '"':
            in_str = True
            continue
        if ch == "'":
            in_chr = True
            continue
        if ch == "{":
            if depth == 0:
                body_start = i
                sig = text[sig_end:i]
                sig_start = sig_end
            depth += 1
        elif ch == "}":
            if depth > 0:
                depth -= 1
                if depth == 0 and body_start is not None:
                    stripped = sig.rstrip()
                    if stripped.endswith(")"):
                        # signature = text back to the previous ; } or start
                        tail = re.split(r"[;}]", stripped)[-1]
                        m = _FUNC_NAME_RE.search(tail)
                        if m and m.group(1) not in ("if", "for", "while", "switch", "return"):
                            name_off = (sig_start + len(stripped) - len(tail)
                                        + m.start(1))
                            yield (m.group(1), text[body_start + 1 : i],
                                   name_off, i)
                    body_start = None
                    sig_end = i + 1
        elif depth == 0 and ch in ";":
            sig_end = i + 1


def _blank_comments(text: str) -> str:
    """Like strip_c_comments, but preserve newlines (stable line numbers)."""

    def repl(m: re.Match) -> str:
        return re.sub(r"[^\n]", " ", m.group(0))

    text = _BLOCK_COMMENT_RE.sub(repl, text)
    text = _LINE_COMMENT_RE.sub(repl, text)
    return _PREPROC_RE.sub(repl, text)


def function_spans(path: Path) -> list[tuple[str, int, int]]:
    """(name, first line, last line) of each function in a C source file."""
    try:
        text = _blank_comments(path.read_text(errors="replace"))
    except OSError:
        return []
    return [
        (name,
         text.count("\n", 0, name_off) + 1,
         text.count("\n", 0, end_off) + 1)
        for name, _, name_off, end_off in _iter_functions(text)
    ]


def is_stub_body(body: str) -> bool:
    statements = [s.strip() for s in body.split(";") if s.strip()]
    if not statements:
        return False
    if not all(_STUB_STMT_RE.match(s) for s in statements):
        return False
    return any(_ENOSYS_STMT_RE.match(s) for s in statements)


def scan_enosys_stubs(lib_posix_dir: Path) -> frozenset[str]:
    """Return names of functions whose entire body is an ENOSYS stub."""
    stubs = set()
    for path in sorted(lib_posix_dir.rglob("*.c")):
        try:
            text = strip_c_comments(path.read_text(errors="replace"))
        except OSError:
            continue
        if "ENOSYS" not in text:
            continue
        for name, body, _, _ in _iter_functions(text):
            if is_stub_body(body):
                stubs.add(name)
    return frozenset(stubs)


def parse_group_tables(option_groups_dir: Path) -> dict[str, dict[str, bool]]:
    """RST page stem -> {function name: supported} from its csv-table rows.

    The Supported column is the curated status: a cell containing "yes"
    means supported; a bare † reference or empty cell means unsupported.
    """
    rosters: dict[str, dict[str, bool]] = {}
    for path in sorted(option_groups_dir.glob("*.rst")):
        stem = path.stem
        if stem == "index":
            continue
        roster: dict[str, bool] = {}
        text = path.read_text()
        for name, cell in _ROW_RE.findall(text) + _SYMBOL_ROW_RE.findall(text):
            supported = "yes" in cell
            # keep the most favorable row if a name repeats across tables
            roster[name] = roster.get(name, False) or supported
        rosters[stem] = roster
    return rosters


_GROUP_REF_RE = re.compile(r"<posix_option_group_\w+>")


def option_group_synonyms(options_dir: Path) -> set[str]:
    """POSIX Options that are synonymous with a Subprofiling Option Group.

    Their doc pages carry no API table and just link to the Option Group
    page, which is where the badges live.
    """
    synonyms = set()
    for rst in sorted(options_dir.glob("*.rst")):
        if rst.stem == "index":
            continue
        text = rst.read_text(encoding="utf-8")
        if ".. csv-table::" not in text and _GROUP_REF_RE.search(text):
            synonyms.add(rst.stem)
    return synonyms


_STATUS_RANK = {"failed": 2, "passed": 1, "skipped": 0}


def _merge_scenarios(scenario_dicts):
    """Merge scenario summaries across contributing groups.

    A variant is reported only when every contributing group ran it
    (intersection), so an aggregate never claims a scenario class that some
    of its interfaces were not tested under; per variant, failed wins.
    Returns None when any contributor has no scenario data at all.
    """
    dicts = list(scenario_dicts)
    if not dicts or any(not d for d in dicts):
        return None
    variants = set(dicts[0])
    for d in dicts[1:]:
        variants &= set(d)
    merged: dict[str, ScenarioSummary] = {}
    for v in sorted(variants):
        entries = [d[v] for d in dicts]
        merged[v] = ScenarioSummary(
            status=max(
                entries, key=lambda s: _STATUS_RANK.get(s.status, 0)
            ).status,
            counts={
                k: sum(e.counts.get(k, 0) for e in entries)
                for k in {k for e in entries for k in e.counts}
            },
            platforms=sorted({p for e in entries for p in e.platforms}),
            failed_platforms=sorted(
                {p for e in entries for p in e.failed_platforms}
            ),
        )
    return merged or None


def _match_component(components, key):
    """Match .codecov.yml components by conventional component_id."""
    for cid in (f"posix_{key}", f"xsi_{key}", key):
        hit = [(pfx, name) for pfx, name, i in components if i == cid]
        if hit:
            return hit
    return None


_IMPL_PREFIX = "modules/lib/posix/lib/posix/"


def _attribute_file(rel_path: str, aliases: dict) -> str | None:
    """Map a workspace-relative source path to a canonical group key."""
    rel = rel_path.replace("\\", "/")
    if _IMPL_PREFIX not in rel:
        return None
    sub = rel.split(_IMPL_PREFIX, 1)[1].split("/")
    if len(sub) < 2:
        return None
    if sub[0] == "options":
        if len(sub) < 3:
            return None
        table = aliases.get("option_dirs") or {}
        key = sub[1]
    else:
        table = aliases.get("lib_dirs") or {}
        key = sub[0]
        if key not in table:
            return None
    return table.get(key, key)  # missing -> identity, explicit null -> None


def group_coverage(coverage_json: dict, aliases: dict) -> dict:
    """Group key -> (line_hits, line_total, [repo-relative dirs])."""
    totals: dict[str, list[int]] = {}
    dirs: dict[str, set] = {}
    for entry in coverage_json.get("files", []):
        # only count implementation sources, not headers
        rel = entry.get("file", "")
        key = _attribute_file(rel, aliases)
        if key is None:
            continue
        # merged shard data may repeat a line; count distinct lines, hit if
        # any record saw it executed (same line-rate Codecov reports)
        by_line: dict[int, bool] = {}
        for ln in entry.get("lines", []):
            n = ln.get("line_number")
            by_line[n] = by_line.get(n, False) or ln.get("count", 0) > 0
        acc = totals.setdefault(key, [0, 0])
        acc[0] += sum(by_line.values())
        acc[1] += len(by_line)
        repo_rel = rel.replace("\\", "/").split(_IMPL_PREFIX, 1)[1]
        dirs.setdefault(key, set()).add("lib/posix/" + repo_rel.rsplit("/", 1)[0])
    return {k: (h, t, sorted(dirs.get(k, ()))) for k, (h, t) in totals.items()}


_UNKNOWN_FN_RE = re.compile(r"^<unknown function \d+>$")


def function_coverage(coverage_json: dict, root: Path | None = None) -> dict:
    """Function name -> (line_hits, line_total, repo-relative file, first line).

    gcovr snapshots sometimes lose function names (recorded as
    ``<unknown function N>``). Such attributions are recovered by locating
    the enclosing function span in the source tree under ``root``. When a
    file is named ``<function>.c`` and carries no *named* attribution even
    after recovery, its whole-file line stats are credited to that function;
    files with any real named attribution are never fallback-attributed.
    """
    per_fn: dict[str, dict] = {}
    fallback: dict[str, tuple] = {}
    for entry in coverage_json.get("files", []):
        rel = entry.get("file", "").replace("\\", "/")
        if _IMPL_PREFIX not in rel:
            continue
        repo_rel = "lib/posix/" + rel.split(_IMPL_PREFIX, 1)[1]
        file_lines: dict[int, bool] = {}
        fn_lines: dict[str, dict[int, bool]] = {}
        for ln in entry.get("lines", []):
            n = ln.get("line_number")
            hit = ln.get("count", 0) > 0
            file_lines[n] = file_lines.get(n, False) or hit
            fn = ln.get("function_name")
            if not fn:
                continue
            d = fn_lines.setdefault(fn, {})
            d[n] = d.get(n, False) or hit
        # re-attribute name-lossy gcovr entries to the enclosing function
        unknown = [fn for fn in fn_lines if _UNKNOWN_FN_RE.match(fn)]
        spans = function_spans(root / repo_rel) if unknown and root else []
        for fn in unknown:
            d = fn_lines.pop(fn)
            real = next(
                (name for name, first, last in spans
                 if any(first <= n <= last for n in d)),
                None,
            )
            if real is None:
                continue
            dst = fn_lines.setdefault(real, {})
            for n, hit in d.items():
                dst[n] = dst.get(n, False) or hit
        for fn, d in fn_lines.items():
            acc = per_fn.setdefault(fn, {"hits": 0, "total": 0, "file": repo_rel,
                                         "line": min(d)})
            acc["hits"] += sum(d.values())
            acc["total"] += len(d)
            acc["line"] = min(acc["line"], min(d))
        if file_lines and not fn_lines:
            fallback[Path(rel).stem] = (sum(file_lines.values()), len(file_lines),
                                        repo_rel, min(file_lines))
    result = {
        k: (v["hits"], v["total"], v["file"], v["line"]) for k, v in per_fn.items()
    }
    for name, stats in fallback.items():
        result.setdefault(name, stats)
    return result


def load_twister_summary(path: Path, aliases: dict):
    """Return (provenance | None, {canonical key: scenarios}, {raw token: scenarios})."""
    data = _load_json(path)
    if not data or data.get("schema_version", 0) > TWISTER_SUMMARY_SCHEMA_VERSION:
        return None, {}, {}
    token_map = aliases.get("twister_tokens") or {}
    scenarios: dict[str, dict[str, ScenarioSummary]] = {}
    raw: dict[str, dict[str, ScenarioSummary]] = {}
    for token, group in (data.get("groups") or {}).items():
        parsed = {}
        for variant, s in (group.get("scenarios") or {}).items():
            parsed[variant] = ScenarioSummary(
                status=s.get("status", "skipped"),
                counts=s.get("counts", {}),
                platforms=s.get("platforms", []),
                failed_platforms=s.get("failed_platforms", []),
            )
        raw[token] = parsed
        key = token_map.get(token, token)  # missing -> identity, null -> skip
        if key is None:
            continue
        scenarios.setdefault(key, {}).update(parsed)
    return data.get("provenance"), scenarios, raw


def load_iso_c(path: Path) -> dict[str, str]:
    """Name -> originating C standard (e.g. "C89"). Accepts the legacy
    list-per-header shape (standard defaults to "C89")."""
    data = _load_yaml(path)
    standards: dict[str, str] = {}
    for header_funcs in (data.get("iso_c") or {}).values():
        if isinstance(header_funcs, dict):
            standards.update({k: str(v) for k, v in header_funcs.items()})
        else:
            standards.update({name: "C89" for name in header_funcs or []})
    return standards


def load_overrides(path: Path) -> dict[str, dict]:
    return _load_yaml(path).get("functions") or {}


def load_codecov_components(module_root: Path) -> list[tuple[str, str, str]]:
    """(path prefix, component name, component id) from .codecov.yml."""
    if yaml is None:
        return []
    try:
        cfg = yaml.safe_load((module_root / ".codecov.yml").read_text())
        components = cfg["component_management"]["individual_components"]
    except (OSError, KeyError, TypeError, yaml.YAMLError):
        return []
    out = []
    for c in components:
        for path in c.get("paths", ()):
            out.append((path.rstrip("*").rstrip("/"), c["name"], c["component_id"]))
    return out


def _find_module_root(start: Path) -> Path:
    for candidate in (start, *start.parents):
        if (candidate / "doc" / "posix" / "option_groups").is_dir():
            return candidate
    raise FileNotFoundError("cannot locate posix module root from " + str(start))


def load_metrics(repo_root: Path | None = None) -> PosixMetrics:
    root = _find_module_root(repo_root or Path(__file__).resolve().parent)
    doc = root / "doc"

    aliases = _load_yaml(doc / "posix_group_aliases.yaml")
    overrides = load_overrides(doc / "posix_api_overrides.yaml")
    iso_c = load_iso_c(doc / "iso_c_functions.yaml")
    stubs = scan_enosys_stubs(root / "lib" / "posix")
    rosters = parse_group_tables(doc / "posix" / "option_groups")

    coverage_json = _load_json(doc / "metrics" / "coverage-posix.json")
    coverage = group_coverage(coverage_json, aliases) if coverage_json else {}
    coverage_provenance = _load_json(doc / "metrics" / "coverage-provenance.json")
    twister_provenance, scenarios, raw_scenarios = load_twister_summary(
        doc / "metrics" / "twister-summary.json", aliases
    )

    # canonical key <-> rst stem (rst_overrides: canonical key -> stem)
    rst_overrides = {
        k: (v or {}).get("rst", k) for k, v in (aliases.get("groups") or {}).items()
    }
    option_aliases = aliases.get("options") or {}
    # canonical key -> twister token to read scenarios from, for groups
    # covered by another group's test suite (e.g. file_system_r)
    twister_overrides = {
        k: v["twister"]
        for k, v in (aliases.get("groups") or {}).items()
        if v and v.get("twister")
    }
    keys = {stem: stem for stem in rosters}
    for key, stem in rst_overrides.items():
        if stem in keys:
            keys[stem] = key

    groups: dict[str, GroupMetrics] = {}
    for stem, roster in rosters.items():
        key = keys[stem]
        implemented = 0
        stub_list = []
        for fn, supported in roster.items():
            status = (overrides.get(fn) or {}).get("status")
            if status in ("implemented", "external"):
                implemented += 1
            elif status in ("stub", "unsupported"):
                stub_list.append(fn)
            elif fn in stubs:
                stub_list.append(fn)
            elif supported:
                implemented += 1
        cov = coverage.get(key)
        groups[key] = GroupMetrics(
            key=key,
            functions_total=len(roster),
            functions_implemented=implemented,
            stub_functions=stub_list,
            coverage=cov[:2] if cov else None,
            coverage_paths=cov[2] if cov else [],
            scenarios=(
                scenarios.get(twister_overrides.get(key, key))
                if twister_provenance
                else None
            ),
        )
    # scenario data may exist for groups whose pages have no tables yet
    if twister_provenance:
        for key, scen in scenarios.items():
            if key not in groups:
                groups[key] = GroupMetrics(key=key, scenarios=scen)

    # POSIX Options: same treatment, keyed by doc/posix/options page stem;
    # coverage paths come from the curated .codecov.yml components
    components = load_codecov_components(root)
    option_rosters = parse_group_tables(doc / "posix" / "options")
    options: dict[str, GroupMetrics] = {}
    option_components: dict[str, list[str]] = {}
    files_index = []
    if coverage_json:
        for entry in coverage_json.get("files", []):
            rel = entry.get("file", "").replace("\\", "/")
            if _IMPL_PREFIX not in rel:
                continue
            by_line: dict[int, bool] = {}
            for ln in entry.get("lines", []):
                n = ln.get("line_number")
                by_line[n] = by_line.get(n, False) or ln.get("count", 0) > 0
            files_index.append(
                ("lib/posix/" + rel.split(_IMPL_PREFIX, 1)[1],
                 sum(by_line.values()), len(by_line))
            )
    synonyms = option_group_synonyms(doc / "posix" / "options")
    for stem, roster in option_rosters.items():
        if stem == "index" or stem in synonyms:
            continue
        implemented = 0
        stub_list = []
        for fn, supported in roster.items():
            status = (overrides.get(fn) or {}).get("status")
            if status in ("implemented", "external"):
                implemented += 1
            elif status in ("stub", "unsupported"):
                stub_list.append(fn)
            elif fn in stubs:
                stub_list.append(fn)
            elif supported:
                implemented += 1
        over = option_aliases.get(stem) or {}
        comp = _match_component(components, stem)
        if not comp:
            # option made up of other groups' implementations (e.g.
            # thread_safe_functions): aggregate their components
            agg = []
            for gk in over.get("coverage") or []:
                agg.extend(_match_component(components, gk) or [])
            comp = agg or None
        cov = None
        paths = []
        if comp:
            option_components[stem] = [name for _, name in comp]
            paths = [pfx for pfx, _ in comp]
        else:
            # option with its own implementation dir but no component
            own = f"lib/posix/options/{stem}"
            if any(rel == own or rel.startswith(own + "/")
                   for rel, _, _ in files_index):
                paths = [own]
        if paths:
            hits = total = 0
            for rel, h, n in files_index:
                if any(rel == pfx or rel.startswith(pfx + "/") or rel.startswith(pfx)
                       for pfx in paths):
                    hits += h
                    total += n
            if total:
                cov = (hits, total)
        scen = None
        if twister_provenance:
            scenario_groups = over.get("scenarios")
            if scenario_groups:
                scen = _merge_scenarios(
                    scenarios.get(twister_overrides.get(gk, gk))
                    for gk in scenario_groups
                )
            else:
                scen = raw_scenarios.get(stem)
        options[stem] = GroupMetrics(
            key=stem,
            functions_total=len(roster),
            functions_implemented=implemented,
            stub_functions=stub_list,
            coverage=cov,
            coverage_paths=paths,
            scenarios=scen,
        )

    # Option Groups that only bundle POSIX Options (the XSI umbrella groups)
    # have no API table of their own; aggregate their implementation
    # completeness over the constituents curated in the alias map (Options
    # first, then Option Groups; constituents without a table contribute
    # nothing).
    for key, over in (aliases.get("groups") or {}).items():
        constituents = (over or {}).get("aggregate") or []
        g = groups.get(key)
        if not constituents or g is None or g.functions_total:
            continue
        stub_union: set[str] = set()
        for name in constituents:
            src = options.get(name) or groups.get(name)
            if src is None:
                continue
            g.functions_total += src.functions_total
            g.functions_implemented += src.functions_implemented
            stub_union.update(src.stub_functions)
        g.stub_functions = sorted(stub_union)

    # per-function metrics over the union of all rosters
    fn_cov = function_coverage(coverage_json, root) if coverage_json else {}
    merged: dict[str, bool] = {}
    for roster in rosters.values():
        for fn, supported in roster.items():
            merged[fn] = merged.get(fn, False) or supported
    # POSIX Option interfaces (mq_*, aio_*, ...) live outside the Option
    # Group rosters but their detail pages carry the same function badges
    for stem, roster in option_rosters.items():
        if stem == "index" or stem in synonyms:
            continue
        for fn, supported in roster.items():
            merged[fn] = merged.get(fn, False) or supported
    # include detected stubs outside the Option Group rosters too (functions
    # implementing POSIX Options, e.g. putmsg), so their Doxygen docs still
    # get an ENOSYS badge
    for fn in stubs:
        merged.setdefault(fn, False)
    functions: dict[str, dict] = {}
    for fn, supported in merged.items():
        status = (overrides.get(fn) or {}).get("status")
        if status not in ("implemented", "external", "stub", "unsupported"):
            if fn in stubs:
                status = "stub"
            else:
                status = "implemented" if supported else "unsupported"
        cov = fn_cov.get(fn)
        functions[fn] = {
            "status": status,
            "coverage": cov[:2] if cov else None,
            "file": cov[2] if cov else None,
            "line": cov[3] if cov else None,
        }

    return PosixMetrics(
        groups=groups,
        options=options,
        option_components=option_components,
        functions=functions,
        iso_c_standards=iso_c,
        iso_c_functions=frozenset(iso_c),
        stub_functions=stubs,
        overrides=overrides,
        coverage_provenance=coverage_provenance,
        twister_provenance=twister_provenance,
        rst_keys=rst_overrides,
    )


def _cli_report(metrics: PosixMetrics, root: Path) -> None:
    """Print the curation worklist: supported functions with no definition.

    These count as implemented on the strength of the curated csv-table
    alone (typically toolchain libc or macro implementations); add them to
    doc/posix_api_overrides.yaml with an explicit status to document why.
    """
    defined: set[str] = set()
    for path in (root / "lib" / "posix").rglob("*.c"):
        text = strip_c_comments(path.read_text(errors="replace"))
        defined.update(name for name, _ in _iter_functions(text))
    rosters = parse_group_tables(root / "doc" / "posix" / "option_groups")
    for stem in sorted(rosters):
        missing = [
            fn
            for fn, supported in rosters[stem].items()
            if supported and fn not in defined and fn not in metrics.overrides
        ]
        if missing:
            print(f"{stem}: supported but no definition or override: {', '.join(missing)}")


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true", help="dump metrics as JSON")
    parser.add_argument(
        "--report", action="store_true", help="print functions needing curation overrides"
    )
    args = parser.parse_args(argv)

    root = _find_module_root(Path(__file__).resolve().parent)
    metrics = load_metrics(root)

    if args.report:
        _cli_report(metrics, root)
        return 0

    if args.json:
        out = {
            key: {
                "functions_total": g.functions_total,
                "functions_implemented": g.functions_implemented,
                "completeness_pct": g.completeness_pct,
                "stub_functions": g.stub_functions,
                "coverage": g.coverage,
                "coverage_pct": g.coverage_pct,
                "scenarios": {
                    v: vars(s) for v, s in (g.scenarios or {}).items()
                },
            }
            for key, g in sorted(metrics.groups.items())
        }
        json.dump(out, sys.stdout, indent=2)
        print()
        return 0

    for key in sorted(metrics.groups):
        g = metrics.groups[key]
        pct = f"{g.completeness_pct:.0f}%" if g.completeness_pct is not None else "n/a"
        cov = f"{g.coverage_pct:.1f}%" if g.coverage_pct is not None else "n/a"
        scen = ",".join(sorted(g.scenarios)) if g.scenarios else "-"
        print(f"{key:32s} impl {pct:>5s}  cov {cov:>6s}  scenarios: {scen}")
        if g.stub_functions:
            print(f"{'':32s} stubs: {', '.join(g.stub_functions)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

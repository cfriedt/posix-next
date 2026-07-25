"""
POSIX Option Group Badges Extension
###################################

Copyright (c) The Zephyr Project Contributors
SPDX-License-Identifier: Apache-2.0

Decorates the POSIX Option Group documentation with data-driven badges:

- Each ``posix/option_groups/<name>`` page gets a badge strip injected after
  its title (``doctree-resolved``): implementation-completeness and line
  coverage donuts plus pass/fail badges for the linux_compat / ubsan / asan /
  static_analysis twister scenario classes.
- The Option Groups index toctree and the per-function "ISO C" pills are
  decorated client-side by ``static/posix-badges.js`` from a JSON database
  written to ``_static/posix-badges.json``.
- Doxygen ``group__posix__option__group__*.html`` pages get an equivalent
  self-contained SVG badge strip injected after the page title
  (``build-finished``), following the rename_posix_headers.py precedent.

All data comes from :mod:`posix_metrics` (scripts/doc/), which reads the
checked-in ``doc/metrics/*.json`` snapshots and the curated YAML files.
Missing data degrades to absent badges; the build never fails for lack of
nightly metrics.

Configuration values
====================

``posix_badges_thresholds``
    ``(green, yellow)`` percentage cutoffs for donut colors. Defaults to the
    ``badge_thresholds`` entry in ``.github/ci-config.json``.
"""

from __future__ import annotations

import hashlib
import json
import re
import sys
from pathlib import Path

from docutils import nodes
from sphinx.application import Sphinx
from sphinx.util import logging
from sphinx.util.display import progress_message

from . import render

try:
    import yaml
except ImportError:  # pragma: no cover
    yaml = None

__version__ = "0.1.0"

logger = logging.getLogger(__name__)

RESOURCES_DIR = Path(__file__).parent / "static"
MODULE_ROOT = Path(__file__).resolve().parents[3]
SECTIONS = (
    ("posix/option_groups/", "groups"),
    ("posix/options/", "options"),
)
OPTION_GROUPS_PREFIX = "posix/option_groups/"

sys.path.insert(0, str(MODULE_ROOT / "scripts" / "doc"))
import posix_metrics  # noqa: E402

_HEADERTITLE_RE = re.compile(
    r'(<div class="headertitle">.*?</div>\s*</div>)', re.DOTALL
)
_EXISTING_BADGES_RE = re.compile(
    r'<div class="pn-badges-doxy"><!-- pn-badges -->.*?</div>', re.DOTALL
)
# a member doc block: its memtitle (function name before "()") through the
# opening of its memdoc div, where the per-function strip is injected
_MEMBER_RE = re.compile(
    r'(<h2 class="memtitle">.*?>(\w+)\(\)</h2>.*?<div class="memdoc">)', re.DOTALL
)
_EXISTING_FUNC_BADGES_RE = re.compile(
    r'<span class="pn-func-badges".*?<!-- /pn-func-badges --></span>', re.DOTALL
)

# module-level cache for the current build
_DB: dict | None = None
_THRESHOLDS = (70.0, 50.0)


def _default_thresholds() -> tuple[float, float]:
    try:
        with (MODULE_ROOT / ".github" / "ci-config.json").open() as f:
            t = json.load(f).get("badge_thresholds", {})
        return float(t.get("green", 70)), float(t.get("yellow", 50))
    except (OSError, ValueError):
        return 70.0, 50.0


def _codecov_components() -> list[tuple[str, str]]:
    """(path-prefix, component name) pairs from .codecov.yml."""
    if yaml is None:
        return []
    try:
        cfg = yaml.safe_load((MODULE_ROOT / ".codecov.yml").read_text())
        components = cfg["component_management"]["individual_components"]
    except (OSError, KeyError, TypeError, yaml.YAMLError):
        return []
    pairs = []
    for c in components:
        for path in c.get("paths", ()):
            pairs.append((path.rstrip("*").rstrip("/"), c["name"]))
    return pairs


def _build_db(codecov_url: str = "") -> dict:
    """Compute the badge database, keyed by RST page stem."""
    from urllib.parse import quote

    metrics = posix_metrics.load_metrics(MODULE_ROOT)
    components = _codecov_components()
    groups = {}
    for key, g in metrics.groups.items():
        url = None
        if codecov_url and g.coverage_paths:
            names = {
                name
                for prefix, name in components
                for d in g.coverage_paths
                if d.startswith(prefix)
            }
            if len(names) == 1:
                component = quote(names.pop(), safe="")
                url = f"{codecov_url}/tree/main?components%5B0%5D={component}"
            else:
                path = g.coverage_paths[0] if len(g.coverage_paths) == 1 else "lib/posix"
                url = f"{codecov_url}/tree/main/{path}"
        groups[metrics.rst_key(key)] = {
            "completeness_pct": g.completeness_pct,
            "coverage_pct": g.coverage_pct,
            "coverage": list(g.coverage) if g.coverage else None,
            "stub_functions": g.stub_functions,
            "codecov_url": url,
            "scenarios": {v: s.status for v, s in (g.scenarios or {}).items()},
        }
    options = {}
    for stem, g in metrics.options.items():
        url = None
        if codecov_url:
            names = metrics.option_components.get(stem)
            if names:
                params = "&".join(
                    f"components%5B{i}%5D={quote(n, safe='')}"
                    for i, n in enumerate(names)
                )
                url = f"{codecov_url}/tree/main?{params}"
            elif len(g.coverage_paths) == 1:
                url = f"{codecov_url}/tree/main/{g.coverage_paths[0]}"
        options[stem] = {
            "completeness_pct": g.completeness_pct,
            "coverage_pct": g.coverage_pct,
            "coverage": list(g.coverage) if g.coverage else None,
            "stub_functions": g.stub_functions,
            "codecov_url": url,
            "scenarios": {v: s.status for v, s in (g.scenarios or {}).items()},
        }

    functions = {}
    for name, f in metrics.functions.items():
        cov = f.get("coverage")
        url = None
        if codecov_url and f.get("file"):
            url = f"{codecov_url}/blob/main/{f['file']}#L{f['line']}"
        functions[name] = {
            "status": f["status"],
            "coverage_pct": (100.0 * cov[0] / cov[1]) if cov and cov[1] else None,
            "coverage": list(cov) if cov else None,
            "codecov_url": url,
        }
    return {
        "groups": groups,
        "options": options,
        "functions": functions,
        "iso_c": dict(sorted(metrics.iso_c_standards.items())),
        "provenance": {
            "coverage": metrics.coverage_provenance,
            "twister": metrics.twister_provenance,
        },
    }


def on_builder_inited(app: Sphinx):
    if RESOURCES_DIR.is_dir():
        app.config.html_static_path.append(str(RESOURCES_DIR))


def _ensure_db(app: Sphinx) -> dict:
    global _DB, _THRESHOLDS
    if _DB is None:
        thresholds = getattr(app.config, "posix_badges_thresholds", None)
        _THRESHOLDS = tuple(thresholds) if thresholds else _default_thresholds()
        codecov = getattr(app.config, "posix_badges_codecov_url", "") or ""
        with progress_message("Building POSIX badge database"):
            _DB = _build_db(codecov.rstrip("/"))
    return _DB


def on_env_get_outdated(app: Sphinx, env, added, changed, removed):
    """Re-resolve Option Group pages whenever the badge data changed.

    The badge strips are baked into the doctrees; without this, pages keep
    stale badges on incremental builds after a metrics snapshot refresh
    (which changes no RST source).
    """
    db = _ensure_db(app)
    digest = hashlib.sha256(
        json.dumps(db, sort_keys=True, ensure_ascii=False).encode("utf-8")
    ).hexdigest()
    previous = getattr(env, "posix_badges_hash", None)
    env.posix_badges_hash = digest
    if previous == digest:
        return []
    skip = set(added) | set(changed) | set(removed)
    return [
        docname
        for docname in env.found_docs
        if any(docname.startswith(prefix) for prefix, _ in SECTIONS)
        and docname not in skip
    ]


def on_env_before_read_docs(app: Sphinx, env, docnames):
    db = _ensure_db(app)

    static_dir = Path(app.outdir) / "_static"
    static_dir.mkdir(parents=True, exist_ok=True)
    out_json = static_dir / "posix-badges.json"
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(db, f, ensure_ascii=False, separators=(",", ":"))
    logger.info(f"posix_badges: wrote {len(db['groups'])} groups to {out_json}")


def on_doctree_resolved(app: Sphinx, doctree, docname):
    """Inject the badge strip after the title of each Option Group page."""
    if _DB is None:
        return
    section = stem = None
    for prefix, sec in SECTIONS:
        if docname.startswith(prefix):
            section, stem = sec, docname[len(prefix):]
            break
    if not section or stem == "index":
        return
    group = _DB[section].get(stem)
    if not group:
        return
    strip = render.badge_strip_html(stem, group, _THRESHOLDS, section=section)
    if not strip:
        return
    for section in doctree.findall(nodes.section):
        for i, child in enumerate(section.children):
            if isinstance(child, nodes.title):
                section.insert(i + 1, nodes.raw("", strip, format="html"))
                return


def _inject_function_badges(text: str, scenarios: dict | None) -> tuple[str, int]:
    """Insert per-function badge strips after each member's memdoc opening.

    ``scenarios`` is the page's Option Group scenario map: the group's
    testsuite covers its functions, so its pass/fail badges are shown on
    every member of that group's page.
    """
    iso = _DB.get("iso_c") or {}
    functions = _DB.get("functions") or {}
    count = 0

    def repl(m):
        nonlocal count
        func = functions.get(m.group(2))
        if not func:
            return m.group(0)
        strip = render.function_badges_html(
            m.group(2), func, iso.get(m.group(2)), _THRESHOLDS, scenarios
        )
        if not strip:
            return m.group(0)
        count += 1
        return m.group(1) + strip

    return _MEMBER_RE.sub(repl, text), count


def on_build_finished(app: Sphinx, exception):
    """Inject SVG badge strips into the Doxygen HTML output.

    Group pages matching an Option Group get a badge strip under the page
    title; every documented function known to the metrics gets an inline
    per-function strip (coverage donut, ENOSYS/ISO C pills) at the top of
    its member doc on any doxygen group page.
    """
    if exception or _DB is None or app.builder.name != "html":
        return
    doxy_html = Path(app.outdir) / "doxygen" / "html"
    if not doxy_html.is_dir():
        return

    group_pages = {
        doxy_html / f"group__{f'posix_option_group_{stem}'.replace('_', '__')}.html":
            ("groups", stem)
        for stem in _DB["groups"]
    }
    group_pages.update({
        doxy_html / f"group__{f'posix_option_{stem}'.replace('_', '__')}.html":
            ("options", stem)
        for stem in _DB["options"]
    })

    pages = 0
    members = 0
    for page in sorted(doxy_html.glob("group__*.html")):
        original = page.read_text(encoding="utf-8")
        # idempotent on incremental builds: drop any previous strips first
        text = _EXISTING_BADGES_RE.sub("", original)
        text = _EXISTING_FUNC_BADGES_RE.sub("", text)

        section_stem = group_pages.get(page)
        scenarios = None
        if section_stem:
            section, stem = section_stem
            scenarios = _DB[section][stem].get("scenarios")
            strip = render.badge_strip_svg(stem, _DB[section][stem], _THRESHOLDS)
            if strip:
                text = _HEADERTITLE_RE.sub(lambda m: m.group(1) + strip, text, count=1)

        text, n = _inject_function_badges(text, scenarios)
        members += n

        if text != original:
            page.write_text(text, encoding="utf-8")
            pages += 1
    logger.info(
        f"posix_badges: injected badges into {pages} Doxygen pages ({members} member docs)"
    )


def setup(app: Sphinx):
    app.add_config_value("posix_badges_thresholds", None, "env")
    app.add_config_value(
        "posix_badges_codecov_url", "https://app.codecov.io/gh/cfriedt/posix-next", "env"
    )

    app.connect("builder-inited", on_builder_inited)
    app.connect("env-get-outdated", on_env_get_outdated)
    app.connect("env-before-read-docs", on_env_before_read_docs)
    app.connect("doctree-resolved", on_doctree_resolved)
    app.connect("build-finished", on_build_finished)

    app.add_css_file("posix-badges.css")
    app.add_js_file("posix-badges.js", defer="defer")

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }

"""
POSIX Option Group Badges Extension
###################################

Copyright (c) The Zephyr Project Contributors
SPDX-License-Identifier: Apache-2.0

Ships the data and assets behind the POSIX Option Group badges; all
rendering happens client-side so the built pages stay byte-stable across
metrics refreshes:

- A JSON badge database is written to ``_static/posix-badges.json`` (groups,
  options, per-function metrics, ISO C standards, provenance, thresholds).
  GitHub Pages serves it compressed and browsers cache it.
- ``static/posix-badges.js`` draws the badge strips, API-table badge
  columns, and index tables from that database on page load (Sphinx pages
  include it via ``add_js_file``).
- Doxygen ``group__*.html`` pages get the same script and stylesheet
  injected at ``build-finished`` (following the rename_posix_headers.py
  precedent), so Option Group pages and member docs are decorated there
  too.

All data comes from :mod:`posix_metrics` (scripts/doc/), which reads the
gitignored ``doc/metrics/*.json`` summaries fetched from CI artifacts and
the curated YAML files. Missing data degrades to absent badges; the build
never fails for lack of nightly metrics.

Configuration values
====================

``posix_badges_thresholds``
    ``(green, yellow)`` percentage cutoffs for donut colors. Defaults to the
    ``badge_thresholds`` entry in ``.github/ci-config.json``.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

from sphinx.application import Sphinx
from sphinx.util import logging
from sphinx.util.display import progress_message

try:
    import yaml
except ImportError:  # pragma: no cover
    yaml = None

__version__ = "0.2.0"

logger = logging.getLogger(__name__)

RESOURCES_DIR = Path(__file__).parent / "static"
MODULE_ROOT = Path(__file__).resolve().parents[3]

sys.path.insert(0, str(MODULE_ROOT / "scripts" / "doc"))
import posix_metrics  # noqa: E402

# assets injected into Doxygen pages, which resolve _static/ from
# doxygen/html/ two levels up
_DOXY_ASSET_TAGS = (
    '<link rel="stylesheet" href="../../_static/posix-badges.css"/>'
    '<script defer src="../../_static/posix-badges.js"></script>'
)

# server-rendered strips from pre-client-side builds, purged from reused
# Doxygen output on incremental builds
_LEGACY_BADGES_RE = re.compile(
    r'<div class="pn-badges-doxy"><!-- pn-badges -->.*?</div>', re.DOTALL
)
_LEGACY_FUNC_BADGES_RE = re.compile(
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


def _build_db(codecov_url: str = "", thresholds=(70.0, 50.0)) -> dict:
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
                path = (
                    g.coverage_paths[0] if len(g.coverage_paths) == 1 else "lib/posix"
                )
                url = f"{codecov_url}/tree/main/{path}"
        groups[metrics.rst_key(key)] = {
            "completeness_pct": g.completeness_pct,
            "coverage_pct": g.coverage_pct,
            "coverage": list(g.coverage) if g.coverage else None,
            "stub_functions": g.stub_functions,
            "codecov_url": url,
            "scenarios": {v: s.status for v, s in (g.scenarios or {}).items()},
            "scenario_failed_functions": {
                v: s.failed_functions
                for v, s in (g.scenarios or {}).items()
                if s.failed_functions is not None
            },
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
            "scenario_failed_functions": {
                v: s.failed_functions
                for v, s in (g.scenarios or {}).items()
                if s.failed_functions is not None
            },
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
        "thresholds": list(thresholds),
        "provenance": {
            "coverage": metrics.coverage_provenance,
            "twister": metrics.twister_provenance,
            **metrics.extra_provenance,
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
            _DB = _build_db(codecov.rstrip("/"), _THRESHOLDS)
    return _DB


def on_env_before_read_docs(app: Sphinx, env, docnames):
    db = _ensure_db(app)

    static_dir = Path(app.outdir) / "_static"
    static_dir.mkdir(parents=True, exist_ok=True)
    out_json = static_dir / "posix-badges.json"
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(db, f, ensure_ascii=False, separators=(",", ":"))
    logger.info(f"posix_badges: wrote {len(db['groups'])} groups to {out_json}")


def on_build_finished(app: Sphinx, exception):
    """Give Doxygen group pages the badge stylesheet and script.

    The injection is data-independent, so Doxygen HTML stays identical
    across metrics refreshes; posix-badges.js does the rendering.
    """
    if exception or app.builder.name != "html":
        return
    doxy_html = Path(app.outdir) / "doxygen" / "html"
    if not doxy_html.is_dir():
        return
    pages = 0
    for page in sorted(doxy_html.glob("group__*.html")):
        text = page.read_text(encoding="utf-8")
        new = _LEGACY_BADGES_RE.sub("", text)
        new = _LEGACY_FUNC_BADGES_RE.sub("", new)
        if "posix-badges.js" not in new:
            new = new.replace("</head>", _DOXY_ASSET_TAGS + "</head>", 1)
        if new != text:
            page.write_text(new, encoding="utf-8")
            pages += 1
    logger.info(f"posix_badges: added badge assets to {pages} Doxygen pages")


def setup(app: Sphinx):
    app.add_config_value("posix_badges_thresholds", None, "env")
    app.add_config_value(
        "posix_badges_codecov_url",
        "https://app.codecov.io/gh/cfriedt/posix-next",
        "env",
    )

    app.connect("builder-inited", on_builder_inited)
    app.connect("env-before-read-docs", on_env_before_read_docs)
    app.connect("build-finished", on_build_finished)

    app.add_css_file("posix-badges.css")
    app.add_js_file("posix-badges.js", defer="defer")

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }

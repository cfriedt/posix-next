"""
POSIX Option Group badge renderers.

Copyright (c) The Zephyr Project Contributors
SPDX-License-Identifier: Apache-2.0

Pure functions turning badge data into HTML (Sphinx pages) or self-contained
SVG (injected into Doxygen HTML). No Sphinx imports, so this module is unit
testable on its own.

Percentage badges reuse the circular donut style of scripts/ci/coverageui.py
(conic-gradient ring with a punched-out center). Status badges are circles
with a glyph and a corner check/cross mark.
"""

from __future__ import annotations

import html

# Scenario classes that map to badges, in display order.
# (key, glyph, label). The static_analysis glyph is a magnifier rather than
# the LLVM dragon logo: the logo is an LLVM Foundation trademark; "LLVM"
# appears in the tooltip text only (nominative use).
SCENARIO_BADGES = (
    ("linux_compat", "\U0001F427", "Linux compatibility tests"),
    ("ubsan", "\u2049\ufe0f", "UBSAN tests"),
    ("asan", "\U0001f4ec", "ASAN tests"),
    ("static_analysis", "\U0001F50D", "Static analysis (LLVM)"),
)

# Light-theme palette from coverageui.py; readable on dark backgrounds too.
_COLORS = {
    "green": "#198754",
    "yellow": "#cc9a06",
    "red": "#dc3545",
    "unknown": "#868e96",
}

_SVG_TRACK = "rgba(128, 140, 160, 0.35)"


def pct_class(pct: float | None, green: float = 70.0, yellow: float = 50.0) -> str:
    if pct is None:
        return "unknown"
    if pct >= green:
        return "green"
    if pct >= yellow:
        return "yellow"
    return "red"


# ---------------------------------------------------------------------------
# HTML (Sphinx pages)
# ---------------------------------------------------------------------------


def _linked(inner: str, href: str | None) -> str:
    if not href:
        return inner
    href = html.escape(href, quote=True)
    return (f'<a class="pn-badge-link" href="{href}" target="_blank"'
            f' rel="noopener">{inner}</a>')


def donut_html(
    pct: float | None, label: str, title: str, thresholds=(70.0, 50.0), href: str | None = None
) -> str:
    cls = pct_class(pct, *thresholds)
    if pct is None:
        text, tooltip = "n/a", f"{title}: no data"
        gradient = "var(--pn-ring-bg) 0% 100%"
    else:
        text, tooltip = f"{pct:.0f}", f"{title}: {pct:.1f}%"
        gradient = f"var(--pn-{cls}) {pct:.4g}%, var(--pn-ring-bg) {pct:.4g}%"
    tooltip = html.escape(tooltip, quote=True)
    ring = (
        f'<div class="pn-badge pn-ring" role="img" title="{tooltip}" aria-label="{tooltip}"'
        f' style="background: conic-gradient({gradient});">'
        f"<span>{text}</span></div>"
    )
    return (
        f'<div class="pn-badge-wrap">'
        + _linked(ring, href)
        + f'<div class="pn-ring-label">{html.escape(label)}</div>'
        + "</div>"
    )


def status_html(key: str, glyph: str, label: str, status: str) -> str:
    passed = status == "passed"
    mark = "✓" if passed else "✗"
    state = "pass" if passed else "fail"
    tooltip = html.escape(f"{label}: {'passing' if passed else 'failing'}", quote=True)
    glyph_cls = " pn-glyph-text" if glyph.isascii() else ""
    return (
        f'<div class="pn-badge-wrap">'
        f'<div class="pn-badge pn-status pn-{state} pn-{key}" role="img"'
        f' title="{tooltip}" aria-label="{tooltip}">'
        f'<span class="pn-glyph{glyph_cls}">{glyph}</span>'
        f'<span class="pn-mark">{mark}</span></div>'
        f"</div>"
    )


def _cells_html(cells: list[str | None], cell_class: str) -> str:
    """Wrap badges in fixed-size cells (an invisible table) so badges align
    in columns across rows; absent badges keep an empty placeholder cell."""
    out = []
    for cell in cells:
        cls = cell_class if cell else f"{cell_class} pn-empty"
        out.append(f'<div class="{cls}">{cell or ""}</div>')
    return "".join(out)


def badge_strip_html(
    stem: str, group: dict, thresholds=(70.0, 50.0), section: str = "groups"
) -> str:
    """Render the full badge strip for one Option Group page.

    ``group`` is the client-JSON shape:
    {"completeness_pct", "coverage_pct", "coverage": [hits, total]|None,
     "scenarios": {variant: status}}
    """
    cells: list[str | None] = []
    if group.get("completeness_pct") is not None:
        cells.append(
            donut_html(
                group["completeness_pct"], "Impl", "Implementation completeness", thresholds
            )
        )
    else:
        cells.append(None)
    cov_title = "Line coverage"
    if group.get("coverage"):
        cov_title += " ({}/{} lines)".format(*group["coverage"])
    if group.get("coverage_pct") is not None:
        cells.append(
            donut_html(
                group["coverage_pct"], "Cov", cov_title, thresholds,
                href=group.get("codecov_url"),
            )
        )
    else:
        cells.append(None)
    scenarios = group.get("scenarios") or {}
    for key, glyph, label in SCENARIO_BADGES:
        status = scenarios.get(key)
        cells.append(
            status_html(key, glyph, label, status) if status in ("passed", "failed") else None
        )
    if not any(cells):
        return ""
    return (
        f'<div class="pn-badges" data-group="{html.escape(stem, quote=True)}"'
        f' data-section="{html.escape(section, quote=True)}">'
        + _cells_html(cells, "pn-cell")
        + "</div>"
    )


# ---------------------------------------------------------------------------
# SVG (Doxygen pages)
# ---------------------------------------------------------------------------

# r chosen so the circle circumference is exactly 100 (stroke-dasharray in %)
_R = 15.9155


def donut_svg(
    pct: float | None, label: str, title: str, thresholds=(70.0, 50.0), href: str | None = None
) -> str:
    color = _COLORS[pct_class(pct, *thresholds)]
    if pct is None:
        text, tooltip, dash = "n/a", f"{title}: no data", "0 100"
    else:
        text, tooltip, dash = f"{pct:.0f}", f"{title}: {pct:.1f}%", f"{pct:.4g} {100 - pct:.4g}"
    tooltip = html.escape(tooltip, quote=True)
    svg = (
        f'<svg class="pn-svg-badge" viewBox="0 0 48 48" width="72" height="72"'
        f' role="img" aria-label="{tooltip}">'
        f"<title>{tooltip}</title>"
        f'<circle cx="24" cy="24" r="{_R}" fill="none" stroke="{_SVG_TRACK}" stroke-width="5"/>'
        f'<circle cx="24" cy="24" r="{_R}" fill="none" stroke="{color}" stroke-width="5"'
        f' stroke-dasharray="{dash}" stroke-dashoffset="25"/>'
        f'<text x="24" y="26" text-anchor="middle" font-size="10" font-weight="700"'
        f' fill="currentColor">{text}</text>'
        f'<text x="24" y="45" text-anchor="middle" font-size="7"'
        f' fill="currentColor" opacity="0.7">{html.escape(label.upper())}</text>'
        f"</svg>"
    )
    return _linked(svg, href)


def status_svg(key: str, glyph: str, label: str, status: str) -> str:
    passed = status == "passed"
    color = _COLORS["green" if passed else "red"]
    mark = "✓" if passed else "✗"
    tooltip = html.escape(f"{label}: {'passing' if passed else 'failing'}", quote=True)
    glyph_size = 10 if glyph.isascii() else 14
    return (
        f'<svg class="pn-svg-badge" viewBox="0 0 48 48" width="72" height="72"'
        f' role="img" aria-label="{tooltip}">'
        f"<title>{tooltip}</title>"
        f'<circle cx="24" cy="24" r="16" fill="none" stroke="{color}" stroke-width="2.5"/>'
        f'<text x="24" y="{24 + glyph_size // 2 - 1}" text-anchor="middle"'
        f' font-size="{glyph_size}" font-weight="700" fill="currentColor">{glyph}</text>'
        f'<text x="39" y="46" text-anchor="middle" font-size="13" font-weight="900"'
        f' fill="{color}">{mark}</text>'
        f"</svg>"
    )


def donut_svg_mini(
    pct: float, title: str, thresholds=(70.0, 50.0), href: str | None = None
) -> str:
    """22px donut for inline per-function badges."""
    color = _COLORS[pct_class(pct, *thresholds)]
    tooltip = html.escape(f"{title}: {pct:.1f}%", quote=True)
    svg = (
        f'<svg viewBox="0 0 40 40" width="32" height="32" role="img"'
        f' aria-label="{tooltip}" style="vertical-align:middle;">'
        f"<title>{tooltip}</title>"
        f'<circle cx="20" cy="20" r="{_R}" fill="none" stroke="{_SVG_TRACK}" stroke-width="6"/>'
        f'<circle cx="20" cy="20" r="{_R}" fill="none" stroke="{color}" stroke-width="6"'
        f' stroke-dasharray="{pct:.4g} {100 - pct:.4g}" stroke-dashoffset="25"/>'
        f'<text x="20" y="24" text-anchor="middle" font-size="13" font-weight="700"'
        f' fill="currentColor">{pct:.0f}</text>'
        f"</svg>"
    )
    return _linked(svg, href)


def _pill_html(text: str, color: str, tooltip: str) -> str:
    tooltip = html.escape(tooltip, quote=True)
    style = (
        "display:inline-block;margin-left:4px;padding:0 5px;border-radius:8px;"
        "font-size:12px;font-weight:700;letter-spacing:.03em;white-space:nowrap;"
        f"vertical-align:middle;border:1px solid {color};color:{color};"
    )
    return (
        f'<span style="{style}" title="{tooltip}" aria-label="{tooltip}">{text}</span>'
    )


def status_svg_mini(glyph: str, label: str, status: str) -> str:
    """22px pass/fail status badge for inline per-function strips."""
    passed = status == "passed"
    color = _COLORS["green" if passed else "red"]
    mark = "✓" if passed else "✗"
    tooltip = html.escape(f"{label}: {'passing' if passed else 'failing'}", quote=True)
    glyph_size = 9 if glyph.isascii() else 12
    return (
        f'<svg viewBox="0 0 40 40" width="32" height="32" role="img"'
        f' aria-label="{tooltip}" style="vertical-align:middle;">'
        f"<title>{tooltip}</title>"
        f'<circle cx="20" cy="20" r="15" fill="none" stroke="{color}" stroke-width="3"/>'
        f'<text x="20" y="{20 + glyph_size // 2}" text-anchor="middle"'
        f' font-size="{glyph_size}" font-weight="700" fill="currentColor">{glyph}</text>'
        f'<text x="34" y="39" text-anchor="middle" font-size="14" font-weight="900"'
        f' fill="{color}">{mark}</text>'
        f"</svg>"
    )


def iso_c_status(scenarios: dict | None) -> str | None:
    """Test status backing the ISO C badge for a group.

    posix-next's ISO C implementations live in the minimal / common C
    libraries, so the ``minimal`` scenario is the closest signal; the base
    scenario is the fallback. Returns "passed", "failed" or None (no data).
    """
    for key in ("minimal", "base"):
        status = (scenarios or {}).get(key)
        if status in ("passed", "failed"):
            return status
    return None


def iso_c_svg(name: str, status: str | None, standard: str = "C") -> str:
    """Round ISO C badge showing the originating C standard (C89, C99, ...)
    with a check/cross mark when the group's test status is known."""
    if status == "passed":
        color, mark, state = _COLORS["green"], "✓", "tests passing"
    elif status == "failed":
        color, mark, state = _COLORS["red"], "✗", "tests failing"
    else:
        color, mark, state = _COLORS["unknown"], "", "no test data"
    tooltip = html.escape(
        f"{name} is part of ISO C since {standard} ({state})", quote=True
    )
    mark_el = (
        f'<text x="34" y="39" text-anchor="middle" font-size="14" font-weight="900"'
        f' fill="{color}">{mark}</text>'
        if mark
        else ""
    )
    font = 11 if len(standard) <= 1 else 9
    return (
        f'<svg viewBox="0 0 40 40" width="32" height="32" role="img"'
        f' aria-label="{tooltip}" style="vertical-align:middle;">'
        f"<title>{tooltip}</title>"
        f'<circle cx="19" cy="19" r="15" fill="none" stroke="{color}" stroke-width="3"/>'
        f'<text x="19" y="22.5" text-anchor="middle" font-size="{font}" font-weight="700"'
        f' fill="currentColor">{html.escape(standard)}</text>'
        f"{mark_el}"
        f"</svg>"
    )


def _cell_inline(content: str | None, wide: bool = False) -> str:
    """Fixed-size inline cell so badges align vertically across members."""
    width = 76 if wide else 40
    style = (
        f"display:inline-flex;width:{width}px;height:36px;"
        "align-items:center;justify-content:center;"
    )
    return f'<span style="{style}">{content or ""}</span>'


_FUNC_BADGES_STYLE = (
    "<style>"
    ".pn-func-badges a{display:inline-flex;text-decoration:none;border:none;color:inherit}"
    ".pn-func-badges svg,.pn-func-badges .pn-jb-pill"
    "{transition:transform .15s ease}"
    ".pn-func-badges svg:hover,.pn-func-badges .pn-jb-pill:hover"
    "{transform:scale(1.3)}"
    "@media (prefers-reduced-motion: reduce)"
    "{.pn-func-badges svg,.pn-func-badges .pn-jb-pill{transition:none}}"
    "</style>"
)


def page_slots(names, functions: dict, iso: dict, scenarios: dict | None) -> dict:
    """Which badge slots have content for any of ``names`` on this page.

    Only slots with content somewhere on the page are reserved, so strips
    align in columns without dead cells for badge classes nothing uses.
    """
    slots = {"cov": False, "scenarios": [], "iso": False, "status": False}
    for key, glyph, label in SCENARIO_BADGES:
        if (scenarios or {}).get(key) in ("passed", "failed"):
            slots["scenarios"].append((key, glyph, label))
    for name in names:
        func = functions.get(name)
        if not func:
            continue
        if func.get("coverage_pct") is not None:
            slots["cov"] = True
        if name in iso:
            slots["iso"] = True
        if func.get("status") in ("stub", "unsupported"):
            slots["status"] = True
    return slots


def function_badges_html(
    name: str,
    func: dict,
    iso_standard: str | None,
    thresholds=(70.0, 50.0),
    scenarios: dict | None = None,
    slots: dict | None = None,
) -> str:
    """Self-contained inline badge strip for one function (Doxygen memdoc).

    Slot order — coverage, scenario classes, ISO C, implementation status —
    with empty placeholder cells so badges align in columns across members.
    Pass ``slots`` (see :func:`page_slots`) to skip classes unused anywhere
    on the page.
    """
    if slots is None:
        slots = {
            "cov": True,
            "scenarios": [
                entry
                for entry in SCENARIO_BADGES
                if (scenarios or {}).get(entry[0]) in ("passed", "failed")
            ],
            "iso": True,
            "status": True,
        }
    cells: list[tuple[str | None, bool]] = []
    any_badge = False

    if slots["cov"]:
        cov = func.get("coverage_pct")
        if cov is not None:
            cells.append(
                (
                    donut_svg_mini(
                        cov, f"{name} line coverage", thresholds,
                        href=func.get("codecov_url"),
                    ),
                    False,
                )
            )
            any_badge = True
        else:
            cells.append((None, False))
    for key, glyph, label in slots["scenarios"]:
        cells.append((status_svg_mini(glyph, label, (scenarios or {})[key]), False))
        any_badge = True
    if slots["iso"]:
        if iso_standard:
            cells.append((iso_c_svg(name, iso_c_status(scenarios), iso_standard), False))
            any_badge = True
        else:
            cells.append((None, False))
    if slots["status"]:
        status = func.get("status")
        if status == "stub":
            pill = _pill_html(
                "ENOSYS", _COLORS["red"], f"{name} is an ENOSYS stub (not implemented)"
            )
            any_badge = True
        elif status == "unsupported":
            pill = _pill_html("✗", _COLORS["red"], f"{name} is not supported")
            any_badge = True
        else:
            pill = None
        cells.append((pill, True))

    if not any_badge:
        return ""
    body = "".join(_cell_inline(content, wide) for content, wide in cells)
    return (
        '<span class="pn-func-badges" style="display:inline-flex;gap:4px;'
        'align-items:center;margin:2px 0 6px;">'
        + _FUNC_BADGES_STYLE
        + body
        + "<!-- /pn-func-badges --></span>"
    )


def badge_strip_svg(stem: str, group: dict, thresholds=(70.0, 50.0)) -> str:
    """Self-contained inline-SVG badge strip for a Doxygen group page."""
    parts = []
    if group.get("completeness_pct") is not None:
        parts.append(
            donut_svg(group["completeness_pct"], "Impl", "Implementation completeness", thresholds)
        )
    if group.get("coverage_pct") is not None:
        parts.append(
            donut_svg(
                group["coverage_pct"], "Cov", "Line coverage", thresholds,
                href=group.get("codecov_url"),
            )
        )
    scenarios = group.get("scenarios") or {}
    for key, glyph, label in SCENARIO_BADGES:
        status = scenarios.get(key)
        if status in ("passed", "failed"):
            parts.append(status_svg(key, glyph, label, status))
    if not parts:
        return ""
    return (
        '<div class="pn-badges-doxy"><!-- pn-badges -->'
        "<style>.pn-badges-doxy{display:flex;gap:16px;align-items:flex-start;"
        "max-width:var(--content-maxwidth,1050px);"
        "margin:12px auto 16px auto;padding:0 var(--spacing-xlarge,34px)}"
        ".pn-badges-doxy a{display:inline-flex;text-decoration:none;border:none;color:inherit}"
        ".pn-badges-doxy .pn-svg-badge{transition:transform .15s ease}"
        ".pn-badges-doxy .pn-svg-badge:hover{transform:scale(1.15)}"
        "@media (prefers-reduced-motion: reduce)"
        "{.pn-badges-doxy .pn-svg-badge{transition:none}}</style>"
        + "".join(parts)
        + "</div>"
    )

# SPDX-License-Identifier: Apache-2.0
#
# posix-next documentation build configuration.
# Reuses Zephyr's Sphinx extensions; ZEPHYR_BASE must be set to the
# Zephyr v4.3.0 tree (e.g. the sibling zephyr/ directory in the west workspace).

import os
import sys
from pathlib import Path

POSIX_NEXT_BASE = Path(__file__).resolve().parent.parent
ZEPHYR_BASE = Path(os.environ.get("ZEPHYR_BASE", str(POSIX_NEXT_BASE.parent / "zephyr"))).resolve()
POSIX_NEXT_BUILD = Path(os.environ.get("OUTPUT_DIR", str(Path(__file__).parent / "_build" / "html"))).resolve()

# Zephyr extension + script paths
sys.path.insert(0, str(ZEPHYR_BASE / "doc" / "_extensions"))
sys.path.insert(0, str(ZEPHYR_BASE / "doc" / "_scripts"))
sys.path.insert(0, str(ZEPHYR_BASE / "scripts"))
sys.path.insert(0, str(ZEPHYR_BASE / "scripts" / "kconfig"))
sys.path.insert(0, str(ZEPHYR_BASE / "scripts" / "west_commands"))

# posix_symbols extension (local)
sys.path.insert(0, str(Path(__file__).parent / "extensions"))

# -- Project -------------------------------------------------------------------

project = "posix-next"
copyright = "2015-2026 Zephyr Project members and individual contributors"
author = "The Zephyr Project Contributors"
version = "0.1.0"
release = version

# -- General -------------------------------------------------------------------

extensions = [
    # Theme
    "sphinx_rtd_theme",
    # Standard Sphinx
    "sphinx.ext.intersphinx",
    "sphinx.ext.graphviz",
    "sphinx.ext.todo",
    # Zephyr extensions needed by the POSIX RST content
    "zephyr.kconfig",       # :kconfig:option: role
    "zephyr.link-roles",    # :zephyr:file:, etc.
    # Lightweight stubs for zephyr.domain / zephyr.application directives
    # (the full extensions require a complete west workspace).
    "zephyr_stubs",
    # Doxygen integration — doxyrunner MUST come before posix_symbols so that
    # Doxygen XML exists when posix_symbols parses it (env-before-read-docs).
    "zephyr.doxyrunner",    # runs Doxygen before Sphinx
    "zephyr.doxybridge",    # .. doxygengroup:: etc. directives
    "zephyr.doxytooltip",   # hover tooltips on Doxygen symbols
    # POSIX symbol search page (local extension)
    "posix_symbols",
    # Quality-of-life
    "sphinx_copybutton",
    "sphinx_tabs.tabs",
]

# Kconfig: generate the local database so that options defined by this module
# (and by the patch series applied to the Zephyr tree) resolve to the local
# kconfig search page. Options only known upstream still resolve through
# Intersphinx.
kconfig_generate_db = True
# ZEPHYR_BASE second: it carries the in-tree Kconfig shims for kconfig-ext
# modules (cmsis_6, hostap, ...) that a local west workspace enumerates.
kconfig_ext_paths = [str(POSIX_NEXT_BASE), str(ZEPHYR_BASE)]

# The zephyr.kconfig extension enumerates modules through west; the docs CI
# builds from a bare checkout with no west workspace, so make sure this
# module is always part of the enumeration (EXTRA_ZEPHYR_MODULES cannot be
# used here: the extension chokes on the PurePosixPath project paths those
# entries produce).
import zephyr_module  # noqa: E402 (needs the sys.path setup above)

_parse_modules = zephyr_module.parse_modules


def _parse_modules_with_posix_next(zephyr_base, *args, **kwargs):
    modules = _parse_modules(zephyr_base, *args, **kwargs)
    if not any(m.meta.get("name") == "posix_next" for m in modules):
        modules += _parse_modules(zephyr_base, modules=[str(POSIX_NEXT_BASE)])
    return modules


zephyr_module.parse_modules = _parse_modules_with_posix_next

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

pygments_style = "sphinx"
highlight_language = "none"
todo_include_todos = False

# -- Intersphinx ---------------------------------------------------------------
# Route external :ref: targets (threads_v2, kconfig, application, …) and
# :kconfig:option: references to the upstream Zephyr 4.3.0 docs.

intersphinx_mapping = {
    "zephyr": ("https://docs.zephyrproject.org/4.3.0/", None),
}

# -- HTML output ---------------------------------------------------------------

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "logo_only": False,
    "navigation_depth": 4,
    "style_external_links": True,
}

html_title = "posix-next"
html_short_title = "posix-next"

# Reuse Zephyr static assets (logo, favicon, CSS overrides) plus our own
html_static_path = [
    str(ZEPHYR_BASE / "doc" / "_static"),
    str(Path(__file__).parent / "_static"),
]
html_css_files = ["posix-next.css"]
html_logo = str(Path(__file__).parent / "_static" / "posix-next-logo.png")
html_favicon = str(ZEPHYR_BASE / "doc" / "_static" / "images" / "favicon.png")

html_baseurl = os.environ.get("DOCS_HTML_BASEURL", "https://cfriedt.github.io/posix-next/")

html_context = {
    "show_license": True,
    "docs_title": "posix-next",
    "is_release": False,
    "display_gh_links": True,
    "reference_links": {
        "Zephyr 4.3.0 docs": "https://docs.zephyrproject.org/4.3.0/",
        "Zephyr POSIX docs": "https://docs.zephyrproject.org/4.3.0/services/portability/posix/index.html",
        "Open Group POSIX": "https://pubs.opengroup.org/onlinepubs/9699919799/",
    },
}

html_show_sourcelink = False
html_domain_indices = False

# -- Doxygen (zephyr.doxyrunner / zephyr.doxybridge) --------------------------

doxyrunner_doxygen = os.environ.get("DOXYGEN_EXECUTABLE", "doxygen")
doxyrunner_projects = {
    "posix": {
        "doxyfile": POSIX_NEXT_BASE / "doc" / "posix.doxyfile.in",
        "outdir": POSIX_NEXT_BUILD / "doxygen",
        "fmt": True,
        "fmt_vars": {
            "ZEPHYR_BASE": str(ZEPHYR_BASE),
            "POSIX_NEXT_BASE": str(POSIX_NEXT_BASE),
            "POSIX_NEXT_VERSION": version,
        },
        "outdir_var": "DOXY_OUT",
    },
}
doxybridge_projects = {"posix": doxyrunner_projects["posix"]["outdir"]}

# posix_symbols: point to the Doxygen XML directory and the deployed HTML path
posix_symbols_doxy_xml_dir = str(doxyrunner_projects["posix"]["outdir"] / "xml")
posix_symbols_doxy_html_url = "../doxygen/html"

# -- Suppress known warnings ---------------------------------------------------
suppress_warnings = [
    "ref.ref",        # unresolved :ref: that Intersphinx also can't find
]


def _install_kconfig_db(app, exception):
    """Put the Kconfig database beside the search page.

    The zephyr.kconfig extension emits the database for a search page at the
    documentation root, but ours lives at posix/kconfig/, and the search
    script fetches "kconfig.json" relative to the page. Move the database
    there and drop the root-level copies.
    """
    import shutil

    if exception is not None or app.builder.name != "html":
        return
    src = Path(app.outdir) / "kconfig" / "kconfig.json"
    if not src.is_file():
        return
    dst = Path(app.outdir) / "posix" / "kconfig" / "kconfig.json"
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, dst)
    (Path(app.outdir) / "kconfig.json").unlink(missing_ok=True)
    src.unlink()
    try:
        src.parent.rmdir()
    except OSError:
        pass


def setup(app):
    app.add_js_file("doxytooltip-patch.js")
    app.connect("build-finished", _install_kconfig_db)

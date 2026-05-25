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

# Kconfig: load the domain (for the role) but skip DB generation in Phase 1
# (no search page yet; DB generation is heavy and needs west modules)
kconfig_generate_db = False
kconfig_ext_paths = [str(POSIX_NEXT_BASE)]

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

# Reuse Zephyr static assets (logo, favicon, CSS overrides)
html_static_path = [
    str(ZEPHYR_BASE / "doc" / "_static"),
]
html_logo = str(ZEPHYR_BASE / "doc" / "_static" / "images" / "kite.png")
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
posix_symbols_doxy_html_url = "doxygen/posix/html"

# -- Suppress known warnings from stripped-down build -------------------------
# :zephyr:code-sample-category: is provided by zephyr.domain which we don't
# load (it crashes without the full west workspace). Suppress the warning.
suppress_warnings = [
    "ref.ref",        # unresolved :ref: that Intersphinx also can't find
]

"""
POSIX Symbol Search Extension
##############################

Copyright (c) The Zephyr Project Contributors
SPDX-License-Identifier: Apache-2.0

Generates a live-filter POSIX API search page (posix-symbols.rst) backed by a
JSON database extracted from the Doxygen XML output.  The page is structurally
identical to Zephyr's ``kconfig.html`` search: an initially blank search box
that filters results in real time and shows the symbol's brief description,
kind, POSIX Option Group membership, and a link to the Open Group specification.

Configuration values
====================

``posix_symbols_doxy_xml_dir``
    Absolute path to the Doxygen XML output directory (e.g. the ``xml/``
    subdirectory under ``doxyrunner_projects["posix"]["outdir"]``).

``posix_symbols_doxy_html_url``
    URL prefix for the Doxygen-generated HTML pages (used to build per-symbol
    links in the search results).  Defaults to ``"../doxygen/html"``.

``posix_symbols_opengroup_base``
    Base URL for Open Group specification links.
    Defaults to ``"https://pubs.opengroup.org/onlinepubs/9699919799"``.
"""

from __future__ import annotations

import json
import re
import shutil
from pathlib import Path
from xml.etree import ElementTree as ET

from docutils import nodes
from sphinx.application import Sphinx
from sphinx.util import logging
from sphinx.util.display import progress_message
from sphinx.util.docutils import SphinxDirective

__version__ = "0.1.0"

logger = logging.getLogger(__name__)

RESOURCES_DIR = Path(__file__).parent / "static"

# ---------------------------------------------------------------------------
# Open Group URL helpers
# ---------------------------------------------------------------------------

OPENGROUP_BASE = "https://pubs.opengroup.org/onlinepubs/9699919799"

# Functions whose names match a simple word lookup in the Open Group functions/
# subdirectory.  Types and macros link to the basedefs page for their header.
_OPENGROUP_FUNCTION_URL = f"{OPENGROUP_BASE}/functions/{{name}}.html"

# Header → basedefs URL segment mapping (used for types & macros)
_HEADER_BASEDEFS: dict[str, str] = {
    "pthread.h": "pthread.h",
    "sched.h": "sched.h",
    "semaphore.h": "semaphore.h",
    "signal.h": "signal.h",
    "posix_signal.h": "signal.h",
    "sys/types.h": "sys_types.h",
    "sys/stat.h": "sys_stat.h",
    "sys/time.h": "sys_time.h",
    "sys/mman.h": "sys_mman.h",
    "sys/select.h": "sys_select.h",
    "sys/socket.h": "sys_socket.h",
    "sys/utsname.h": "sys_utsname.h",
    "unistd.h": "unistd.h",
    "fcntl.h": "fcntl.h",
    "poll.h": "poll.h",
    "dirent.h": "dirent.h",
    "mqueue.h": "mqueue.h",
    "time.h": "time.h",
    "posix_time.h": "time.h",
    "posix_limits.h": "limits.h",
    "posix_stdlib.h": "stdlib.h",
    "posix_string.h": "string.h",
    "fnmatch.h": "fnmatch.h",
    "syslog.h": "syslog.h",
    "pwd.h": "pwd.h",
    "grp.h": "grp.h",
    "netdb.h": "netdb.h",
    "netinet/in.h": "netinet_in.h",
    "netinet/tcp.h": "netinet_tcp.h",
    "arpa/inet.h": "arpa_inet.h",
    "net/if.h": "net_if.h",
    "aio.h": "aio.h",
}


def _opengroup_url(kind: str, name: str, header: str) -> str:
    """Return an Open Group specification URL for a symbol."""
    if kind == "function":
        return _OPENGROUP_FUNCTION_URL.format(name=name)
    base = _HEADER_BASEDEFS.get(header, "")
    if base:
        return f"{OPENGROUP_BASE}/basedefs/{base}.html"
    return ""


# ---------------------------------------------------------------------------
# Doxygen XML parser
# ---------------------------------------------------------------------------

# Map Doxygen memberdef/@kind to a human-readable label
_KIND_LABEL = {
    "function": "function",
    "typedef": "type",
    "define": "macro",
    "struct": "struct",
    "union": "union",
    "enum": "enum",
    "variable": "variable",
}


def _text(element: ET.Element | None) -> str:
    """Return all text content inside an XML element, stripping whitespace."""
    if element is None:
        return ""
    return " ".join("".join(element.itertext()).split())


def _parse_xml_dir(xml_dir: Path) -> list[dict]:
    """
    Parse a Doxygen XML directory and return a list of symbol dicts.

    Each symbol has:
        name, kind, brief, group, group_label, header, doxy_refid
    """
    index_xml = xml_dir / "index.xml"
    if not index_xml.exists():
        logger.warning(
            f"posix_symbols: Doxygen XML index not found at {index_xml}. "
            "Have you run the Doxygen build step?"
        )
        return []

    tree = ET.parse(index_xml)
    root = tree.getroot()

    symbols: list[dict] = []

    for compound in root.findall("compound"):
        kind = compound.get("kind", "")
        refid = compound.get("refid", "")
        if kind not in ("group", "file"):
            continue

        compound_xml = xml_dir / f"{refid}.xml"
        if not compound_xml.exists():
            continue

        ctree = ET.parse(compound_xml)
        croot = ctree.getroot()

        for cdef in croot.findall("compounddef"):
            cdef_kind = cdef.get("kind", "")

            # For groups, record the group name and title
            if cdef_kind == "group":
                group_name = _text(cdef.find("compoundname"))
                group_title = _text(cdef.find("title")) or group_name
            else:
                group_name = ""
                group_title = ""

            for sectiondef in cdef.findall("sectiondef"):
                for memberdef in sectiondef.findall("memberdef"):
                    m_kind = memberdef.get("kind", "")
                    if m_kind not in _KIND_LABEL:
                        continue

                    name = _text(memberdef.find("name"))
                    if not name:
                        continue

                    brief = _text(memberdef.find("briefdescription"))
                    # strip leading "More..." / trailing punctuation noise
                    brief = re.sub(r"\s*More\.\.\.\s*$", "", brief).strip()

                    location = memberdef.find("location")
                    header_file = ""
                    if location is not None:
                        header_file = location.get("file", "")
                        # strip leading path up to zephyr/posix/
                        m = re.search(r"(?:include/)?zephyr/posix/(.+)", header_file)
                        if m:
                            header_file = m.group(1)

                    doxy_refid = memberdef.get("id", "")

                    symbols.append(
                        {
                            "name": name,
                            "kind": _KIND_LABEL[m_kind],
                            "brief": brief,
                            "group": group_name,
                            "group_label": group_title,
                            "header": header_file,
                            "doxy_refid": doxy_refid,
                        }
                    )

    # De-duplicate by name+kind (a symbol can appear in both file and group XML)
    seen: set[str] = set()
    unique: list[dict] = []
    for s in symbols:
        key = f"{s['name']}::{s['kind']}"
        if key not in seen:
            seen.add(key)
            unique.append(s)

    # Sort: put group-attached symbols first (they have richer metadata)
    unique.sort(key=lambda s: (not bool(s["group"]), s["name"].lower()))

    return unique


# ---------------------------------------------------------------------------
# JSON database builder
# ---------------------------------------------------------------------------


def _build_db(app: Sphinx) -> list[dict]:
    """Build the symbol database from Doxygen XML and return as a list."""
    xml_dir = Path(getattr(app.config, "posix_symbols_doxy_xml_dir", ""))
    if not xml_dir.is_dir():
        # Try to derive from doxyrunner_projects if available
        projects = getattr(app.config, "doxyrunner_projects", {})
        if "posix" in projects:
            xml_dir = Path(projects["posix"]["outdir"]) / "xml"

    if not xml_dir.is_dir():
        logger.warning(
            "posix_symbols: XML directory not found. "
            "Set posix_symbols_doxy_xml_dir in conf.py."
        )
        return []

    raw = _parse_xml_dir(xml_dir)
    opengroup_base = getattr(app.config, "posix_symbols_opengroup_base", OPENGROUP_BASE)

    for sym in raw:
        sym["opengroup_url"] = _opengroup_url(sym["kind"], sym["name"], sym["header"])

    return raw


# ---------------------------------------------------------------------------
# Sphinx extension hooks
# ---------------------------------------------------------------------------


class PosixSearchDirective(SphinxDirective):
    """Insert the POSIX symbol search container."""

    has_content = False
    required_arguments = 0
    optional_arguments = 0

    def run(self):
        container = nodes.raw(
            "",
            '<div id="__posix-search"></div>',
            format="html",
        )
        return [container]


def on_env_before_read_docs(app: Sphinx, env, docnames):
    """
    Build the POSIX symbols JSON database.

    This hook fires after ``builder-inited`` (where doxyrunner runs Doxygen),
    so the Doxygen XML output is guaranteed to exist.
    """
    with progress_message("Building POSIX symbol database"):
        symbols = _build_db(app)

    doxy_html_url = getattr(
        app.config, "posix_symbols_doxy_html_url", "../doxygen/html"
    )

    db = {
        "symbols": symbols,
        "doxy_html_url": doxy_html_url,
    }

    # Write JSON to the Sphinx output _static directory
    static_dir = Path(app.outdir) / "_static"
    static_dir.mkdir(parents=True, exist_ok=True)
    out_json = static_dir / "posix-symbols.json"
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(db, f, ensure_ascii=False, indent=None, separators=(",", ":"))

    logger.info(f"posix_symbols: wrote {len(symbols)} symbols to {out_json}")


def on_builder_inited(app: Sphinx):
    """Copy static assets (JS, CSS) to the Sphinx output directory."""
    # Assets are copied by Sphinx automatically via html_static_path,
    # but we add RESOURCES_DIR here for convenience.
    if RESOURCES_DIR.is_dir():
        app.config.html_static_path.append(str(RESOURCES_DIR))


def setup(app: Sphinx):
    app.add_config_value("posix_symbols_doxy_xml_dir", "", "env")
    app.add_config_value("posix_symbols_doxy_html_url", "../doxygen/html", "env")
    app.add_config_value("posix_symbols_opengroup_base", OPENGROUP_BASE, "env")

    app.add_directive("posix-search", PosixSearchDirective)

    app.connect("builder-inited", on_builder_inited)
    app.connect("env-before-read-docs", on_env_before_read_docs)

    # Add CSS and JS
    app.add_css_file("posix-symbols.css")
    app.add_js_file("posix-symbols.mjs", type="module")

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }

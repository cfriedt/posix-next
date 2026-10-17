"""
In-browser demo directive
#########################

Copyright The Zephyr Project Contributors
SPDX-License-Identifier: Apache-2.0

``.. web-demo::`` puts a Zephyr image running under QEMU compiled to
WebAssembly on a page: a poster with a play button that becomes a terminal
in the same frame. The directive's content is the caption under it.

Options:

``:bundle:``
    Site-relative directory of the bundle scripts/web-demo/pack.py produced
    (default ``_static/demo/``). A build without it shows the poster and
    says so.
``:poster:``
    Site-relative poster image (default ``_static/web-demo-poster.svg``).
``:button:``
    The play button's label.

The page side is ``_static/web-demo.js`` and ``_static/web-demo.css``; the
service worker ``web-demo-sw.js`` at the site root supplies the isolation
headers the emulator's shared memory needs. The directive emits every path
relative to the page, so it works at any depth of the site.
"""

from __future__ import annotations

from docutils import nodes
from docutils.parsers.rst import directives
from sphinx.application import Sphinx
from sphinx.util.docutils import SphinxDirective
from sphinx.util.osutil import relative_uri


class web_demo(nodes.General, nodes.Element):
    pass


class WebDemo(SphinxDirective):
    has_content = True
    option_spec = {
        "bundle": directives.unchanged,
        "poster": directives.unchanged,
        "button": directives.unchanged,
    }

    def run(self) -> list[nodes.Node]:
        node = web_demo()
        node["bundle"] = self.options.get("bundle", "_static/demo/")
        node["poster"] = self.options.get("poster", "_static/web-demo-poster.svg")
        node["button"] = self.options.get("button", "Run it in your browser")
        self.state.nested_parse(self.content, self.content_offset, node)
        return [node]


def visit_web_demo_html(self, node: web_demo) -> None:
    page = self.builder.get_target_uri(self.builder.current_docname)

    def rel(path: str) -> str:
        return relative_uri(page, path)

    button = self.encode(node["button"])
    self.body.append(
        f'<link rel="stylesheet" href="{rel("_static/web-demo.css")}">\n'
        f'<div class="pn-demo" data-base="{rel(node["bundle"])}" '
        f'data-sw="{rel("web-demo-sw.js")}">\n'
        '<div class="pn-demo-frame">\n'
        f'<img class="pn-demo-poster" src="{rel(node["poster"])}" width="960" height="540" '
        'alt="the terminal of the running image">\n'
        f'<button class="pn-demo-play" type="button" aria-label="{button}">\n'
        '<svg viewBox="0 0 100 100" aria-hidden="true">'
        '<circle cx="50" cy="50" r="47"/><path d="M40 29 L74 50 L40 71 Z"/></svg>\n'
        f"<span>{button}</span>\n</button>\n"
        '<div class="pn-demo-term" hidden></div>\n'
        '<div class="pn-demo-controls" hidden>\n'
        '<button class="pn-demo-max" type="button" title="Fill the tab">&#x2922;</button>\n'
        '<button class="pn-demo-close" type="button" title="Back to the page">&#x2715;</button>\n'
        "</div>\n"
        '<div class="pn-demo-status" hidden></div>\n'
        "</div>\n"
        '<div class="pn-demo-caption">\n'
    )


def depart_web_demo_html(self, node: web_demo) -> None:
    page = self.builder.get_target_uri(self.builder.current_docname)
    self.body.append(
        f'</div>\n</div>\n<script defer src="{relative_uri(page, "_static/web-demo.js")}"></script>\n'
    )


def skip(self, node: web_demo) -> None:
    raise nodes.SkipNode


def setup(app: Sphinx) -> dict:
    app.add_node(
        web_demo,
        html=(visit_web_demo_html, depart_web_demo_html),
        latex=(skip, None),
        text=(skip, None),
        man=(skip, None),
        texinfo=(skip, None),
    )
    app.add_directive("web-demo", WebDemo)
    return {"version": "1.0", "parallel_read_safe": True, "parallel_write_safe": True}

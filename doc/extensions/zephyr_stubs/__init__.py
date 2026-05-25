# Copyright (c) 2025 The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0

"""
Lightweight stubs for Zephyr Sphinx directives and roles.

The full ``zephyr.domain`` and ``zephyr.application`` extensions require a
complete west workspace (board catalog, devicetree bindings, west module
enumeration, …).  This module provides the minimal subset needed by the
posix-next standalone docs build:

* ``.. zephyr:code-sample::``      – renders a sample page with title
* ``.. zephyr:code-sample-category::`` – renders a category listing with toctree
* ``:zephyr:code-sample:``         – inline cross-reference role (renders as text)
* ``.. zephyr-app-commands::``     – renders ``west build`` shell snippets
"""

from __future__ import annotations

from typing import Any

from docutils import nodes
from docutils.parsers.rst import Directive, directives
from docutils.statemachine import StringList
from sphinx import addnodes
from sphinx.application import Sphinx
from sphinx.domains import Domain, ObjType
from sphinx.roles import XRefRole
from sphinx.transforms import SphinxTransform
from sphinx.transforms.post_transforms import SphinxPostTransform
from sphinx.util.docutils import SphinxDirective
from sphinx.util.nodes import make_refnode, NodeMatcher


# ---------------------------------------------------------------------------
# Placeholder nodes
# ---------------------------------------------------------------------------

class _CodeSampleNode(nodes.Element):
    """Replaced by _ConvertCodeSampleNode transform."""
    pass


class _CodeSampleListingPlaceholder(nodes.Element):
    pass


# ---------------------------------------------------------------------------
# code-sample
# ---------------------------------------------------------------------------

class CodeSampleDirective(SphinxDirective):
    """``.. zephyr:code-sample:: <id>`` – marks a document as a code sample."""

    required_arguments = 1
    optional_arguments = 0
    option_spec = {
        "name": directives.unchanged,
        "relevant-api": directives.unchanged,
    }
    has_content = True

    def run(self):
        sample_id = self.arguments[0]
        name = self.options.get("name", sample_id)
        env = self.state.document.settings.env

        domain = env.get_domain("zephyr")
        domain.data["code-samples"][sample_id] = {
            "id": sample_id,
            "name": name,
            "docname": env.docname,
            "description": " ".join(self.content) if self.content else "",
        }

        node = _CodeSampleNode()
        node["id"] = sample_id
        node["name"] = name

        desc = nodes.container()
        self.state.nested_parse(self.content, self.content_offset, desc)
        node += desc

        return [node]


# ---------------------------------------------------------------------------
# Transform: convert _CodeSampleNode → section, absorbing all siblings
# ---------------------------------------------------------------------------

class _ConvertCodeSampleNode(SphinxTransform):
    default_priority = 100

    def apply(self):
        for node in list(self.document.findall(_CodeSampleNode)):
            parent = node.parent
            if parent is None:
                continue

            idx = parent.index(node)
            siblings = parent.children[idx + 1:]

            section = nodes.section(ids=[node["id"]])
            section += nodes.title(text=node["name"])
            section += node.children      # description
            section.extend(siblings)       # Overview, Building, etc.

            node.replace_self(section)
            for sib in siblings:
                parent.remove(sib)


# ---------------------------------------------------------------------------
# code-sample-category
# ---------------------------------------------------------------------------

class CodeSampleCategoryDirective(SphinxDirective):
    """``.. zephyr:code-sample-category:: <id>`` – groups code samples."""

    required_arguments = 1
    optional_arguments = 0
    option_spec = {
        "name": directives.unchanged,
        "show-listing": directives.flag,
        "live-search": directives.flag,
        "glob": directives.unchanged,
    }
    has_content = True
    final_argument_whitespace = True

    def run(self):
        cat_id = self.arguments[0]
        name = self.options.get("name", cat_id)
        glob_pattern = self.options.get("glob", "*/*")

        section = nodes.section(ids=[cat_id])
        section += nodes.title(text=name)

        desc = nodes.container()
        self.state.nested_parse(self.content, self.content_offset, desc)
        section += desc

        if "show-listing" in self.options:
            section += _CodeSampleListingPlaceholder()

        toctree_rst = StringList(
            [
                ".. toctree::",
                "   :titlesonly:",
                "   :glob:",
                "   :hidden:",
                "   :maxdepth: 1",
                "",
                f"   {glob_pattern}",
            ],
            source=self.state.document.settings.env.docname,
        )
        toctree_container = nodes.container()
        self.state.nested_parse(toctree_rst, self.content_offset, toctree_container)
        section += toctree_container

        return [section]


# ---------------------------------------------------------------------------
# Post-transform: replace listing placeholder with real sample list
# ---------------------------------------------------------------------------

class _ResolveSampleListing(SphinxPostTransform):
    default_priority = 5

    def run(self, **kwargs: Any) -> None:
        for node in list(self.document.findall(_CodeSampleListingPlaceholder)):
            samples = sorted(
                self.env.domaindata["zephyr"]["code-samples"].values(),
                key=lambda s: s["name"].casefold(),
            )
            if not samples:
                node.replace_self([])
                continue

            dl = nodes.definition_list(classes=["code-sample-listing"])
            for sample in samples:
                dli = nodes.definition_list_item()

                term = nodes.term()
                ref = nodes.reference("", sample["name"], internal=True,
                                      refuri=self.app.builder.get_relative_uri(
                                          self.env.docname, sample["docname"]))
                term += ref
                dli += term

                if sample.get("description"):
                    dd = nodes.definition()
                    dd += nodes.paragraph(text=sample["description"])
                    dli += dd

                dl += dli

            node.replace_self([dl])


# ---------------------------------------------------------------------------
# zephyr-app-commands
# ---------------------------------------------------------------------------

class ZephyrAppCommandsDirective(Directive):
    """``.. zephyr-app-commands::`` – renders west build / flash commands."""

    has_content = False
    required_arguments = 0
    optional_arguments = 0
    option_spec = {
        "tool": directives.unchanged,
        "app": directives.unchanged,
        "zephyr-app": directives.unchanged,
        "cd-into": directives.flag,
        "generator": directives.unchanged,
        "host-os": directives.unchanged,
        "board": directives.unchanged,
        "shield": directives.unchanged,
        "conf": directives.unchanged,
        "gen-args": directives.unchanged,
        "build-args": directives.unchanged,
        "snippets": directives.unchanged,
        "build-dir": directives.unchanged,
        "build-dir-fmt": directives.unchanged,
        "goals": directives.unchanged_required,
        "maybe-skip-config": directives.flag,
        "compact": directives.flag,
        "west-args": directives.unchanged,
        "flash-args": directives.unchanged,
        "debug-args": directives.unchanged,
        "debugserver-args": directives.unchanged,
        "attach-args": directives.unchanged,
    }

    def run(self):
        app = self.options.get("app") or self.options.get("zephyr-app", ".")
        board = self.options.get("board", "<board>")
        goals = self.options.get("goals", "build").split()

        lines = [f"west build -b {board} {app}"]
        for goal in goals:
            if goal == "build":
                continue
            lines.append(f"west {goal}")

        content = "\n".join(lines)
        literal = nodes.literal_block(content, content)
        literal["language"] = "console"
        return [literal]


# ---------------------------------------------------------------------------
# Minimal zephyr domain
# ---------------------------------------------------------------------------

class ZephyrDomain(Domain):
    name = "zephyr"
    label = "Zephyr"

    roles = {
        "code-sample": XRefRole(innernodeclass=nodes.inline, warn_dangling=False),
        "code-sample-category": XRefRole(innernodeclass=nodes.inline, warn_dangling=False),
    }

    directives = {
        "code-sample": CodeSampleDirective,
        "code-sample-category": CodeSampleCategoryDirective,
    }

    object_types = {
        "code-sample": ObjType("code sample", "code-sample"),
        "code-sample-category": ObjType("code sample category", "code-sample-category"),
    }

    initial_data: dict[str, Any] = {
        "code-samples": {},
    }

    def clear_doc(self, docname: str) -> None:
        self.data["code-samples"] = {
            k: v for k, v in self.data["code-samples"].items()
            if v["docname"] != docname
        }

    def merge_domaindata(self, docnames: list[str], otherdata: dict) -> None:
        self.data["code-samples"].update(otherdata["code-samples"])

    def resolve_xref(self, env, fromdocname, builder, type, target, node, contnode):
        if type == "code-sample":
            sample = self.data["code-samples"].get(target)
            if sample:
                return make_refnode(
                    builder, fromdocname, sample["docname"],
                    sample["id"], contnode, sample.get("description", ""),
                )
        return None

    def get_objects(self):
        for sample in self.data["code-samples"].values():
            yield (
                sample["id"], sample["name"], "code-sample",
                sample["docname"], sample["id"], 1,
            )


# ---------------------------------------------------------------------------
# setup
# ---------------------------------------------------------------------------

def setup(app: Sphinx):
    app.add_domain(ZephyrDomain)
    app.add_directive("zephyr-app-commands", ZephyrAppCommandsDirective)
    app.add_transform(_ConvertCodeSampleNode)
    app.add_post_transform(_ResolveSampleListing)
    app.add_node(_CodeSampleNode)
    app.add_node(_CodeSampleListingPlaceholder)

    return {
        "version": "0.2.0",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }

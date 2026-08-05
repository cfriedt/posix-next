/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 *
 * Client-side rendering for the POSIX Option Group badges. Pages carry no
 * baked-in badge data: everything is drawn at load time from the JSON
 * database _static/posix-badges.json written by the posix_badges extension,
 * so the HTML stays byte-stable across metrics refreshes.
 *
 * - Option Group / Option detail pages get the badge strip after the title
 *   and a badge column added to their API tables (coverage, linux_compat,
 *   userspace, ubsan, asan, static_analysis, ISO C standard, ENOSYS).
 * - The option_groups / options index toctrees are re-rendered as tables
 *   with aligned Status columns.
 * - Doxygen group__*.html pages (which include this script via the
 *   extension's build-finished hook) get the strip after the page title and
 *   an inline per-function strip at the top of each member doc.
 */
(function () {
  "use strict";

  /* resolve _static/ from this script's own URL (works at any page depth) */
  var script = document.currentScript;
  var staticBase = script
    ? script.src.replace(/posix-badges\.js.*$/, "")
    : "_static/";

  var SCENARIO_BADGES = [
    ["linux_compat", "\u{1F427}", "Linux compatibility tests"],
    ["userspace", "\u{1F9D1}\u{200D}\u{1F4BB}", "Userspace tests"],
    ["ubsan", "\u{2049}\u{FE0F}", "UBSAN tests"],
    ["asan", "\u{1F4EC}", "ASAN tests"],
    ["static_analysis", "\u{1F50D}", "Static analysis (clang scan-build)"],
  ];

  /* (green, yellow) percentage cutoffs; replaced by db.thresholds */
  var thresholds = [70, 50];

  function pctClass(pct) {
    if (pct === null || pct === undefined) {
      return "unknown";
    }
    return pct >= thresholds[0] ? "green" : pct >= thresholds[1] ? "yellow" : "red";
  }

  function indexSection() {
    if (/\/option_groups\/(index\.html)?$/.test(window.location.pathname)) {
      return "groups";
    }
    if (/\/options\/(index\.html)?$/.test(window.location.pathname)) {
      return "options";
    }
    return null;
  }

  /* Option Group / Option detail page, from the URL alone */
  function detailPage() {
    var m = window.location.pathname.match(
      /\/posix\/(option_groups|options)\/([^/]+)\.html$/
    );
    if (!m || m[2] === "index") {
      return null;
    }
    return {section: m[1] === "options" ? "options" : "groups", stem: m[2]};
  }

  /* Doxygen group page: group__<name-with-underscores-doubled>.html.
     Pages for an Option Group / Option carry a section and stem; other
     group pages still get per-member strips (section: null). */
  function doxygenPage() {
    var seg = window.location.pathname.split("/").pop() || "";
    var m = seg.match(/^group__(.+)\.html$/);
    if (!m) {
      return null;
    }
    var name = m[1].replace(/__/g, "_");
    if (name.indexOf("posix_option_group_") === 0) {
      return {section: "groups", stem: name.slice("posix_option_group_".length)};
    }
    if (name.indexOf("posix_option_") === 0) {
      return {section: "options", stem: name.slice("posix_option_".length)};
    }
    return {section: null, stem: null};
  }

  function fetchDb() {
    return fetch(staticBase + "posix-badges.json").then(function (r) {
      if (!r.ok) {
        throw new Error("posix-badges.json: HTTP " + r.status);
      }
      return r.json();
    });
  }

  /* one fixed-size invisible cell of the badge "table" */
  function cell(child, wide) {
    var el = document.createElement("span");
    el.className = "pn-cell" + (wide ? " pn-wide" : "") + (child ? "" : " pn-empty");
    if (child) {
      el.appendChild(child);
    }
    return el;
  }

  function linked(el, href) {
    if (!href) {
      return el;
    }
    var a = document.createElement("a");
    a.className = "pn-badge-link";
    a.href = href;
    a.target = "_blank";
    a.rel = "noopener";
    a.appendChild(el);
    return a;
  }

  function miniDonut(pct, title, href) {
    var ring = document.createElement("span");
    ring.className = "pn-badge pn-ring";
    ring.setAttribute("role", "img");
    ring.style.background =
      "conic-gradient(var(--pn-" + pctClass(pct) + ") " + pct +
      "%, var(--pn-ring-bg) " + pct + "%)";
    ring.title = title + ": " + pct.toFixed(1) + "%";
    ring.setAttribute("aria-label", ring.title);
    var span = document.createElement("span");
    span.textContent = String(Math.round(pct));
    ring.appendChild(span);
    return linked(ring, href);
  }

  /* labeled donut for the page-title strips (Impl / Cov) */
  function labeledDonut(pct, label, title, href) {
    var wrap = document.createElement("div");
    wrap.className = "pn-badge-wrap";
    wrap.appendChild(miniDonut(pct, title, href));
    var caption = document.createElement("div");
    caption.className = "pn-ring-label";
    caption.textContent = label;
    wrap.appendChild(caption);
    return wrap;
  }

  function miniStatus(key, glyph, label, status) {
    var badge = document.createElement("span");
    var passed = status === "passed";
    badge.className =
      "pn-badge pn-status pn-" + (passed ? "pass" : "fail") + " pn-" + key;
    badge.setAttribute("role", "img");
    badge.title = label + ": " + (passed ? "passing" : "failing");
    badge.setAttribute("aria-label", badge.title);
    var g = document.createElement("span");
    g.className = "pn-glyph" + (/^[\x00-\x7F]+$/.test(glyph) ? " pn-glyph-text" : "");
    g.textContent = glyph;
    var mark = document.createElement("span");
    mark.className = "pn-mark";
    mark.textContent = passed ? "✓" : "✗";
    badge.appendChild(g);
    badge.appendChild(mark);
    return badge;
  }

  function pill(text, cls, tooltip) {
    var el = document.createElement("span");
    el.className = "pn-pill " + cls;
    el.textContent = text;
    el.title = tooltip;
    el.setAttribute("aria-label", tooltip);
    return el;
  }

  /* ISO C implementations live in the minimal/common C libraries, so the
     "minimal" scenario is the closest test signal; base is the fallback. */
  function isoCStatus(scenarios) {
    var keys = ["minimal", "base"];
    for (var i = 0; i < keys.length; i++) {
      var s = scenarios[keys[i]];
      if (s === "passed" || s === "failed") {
        return s;
      }
    }
    return null;
  }

  function isoCBadge(name, standard, status) {
    var badge = document.createElement("span");
    var cls = status === "passed" ? "pn-pass" : status === "failed" ? "pn-fail" : "pn-none";
    badge.className = "pn-badge pn-status pn-isoc-round " + cls;
    badge.setAttribute("role", "img");
    var state =
      status === "passed" ? "tests passing" : status === "failed" ? "tests failing" : "no test data";
    badge.title = name + " is part of ISO C since " + standard + " (" + state + ")";
    badge.setAttribute("aria-label", badge.title);
    var g = document.createElement("span");
    g.className = "pn-glyph pn-glyph-text";
    g.textContent = standard;
    badge.appendChild(g);
    if (status === "passed" || status === "failed") {
      var mark = document.createElement("span");
      mark.className = "pn-mark";
      mark.textContent = status === "passed" ? "✓" : "✗";
      badge.appendChild(mark);
    }
    return badge;
  }

  /* full badge strip for one group/option: implementation and coverage
     donuts plus one pass/fail badge per scenario class, in fixed slots */
  function buildGroupStrip(stem, group, section) {
    var cells = [];
    if (group.completeness_pct !== null && group.completeness_pct !== undefined) {
      cells.push(
        labeledDonut(group.completeness_pct, "Impl", "Implementation completeness")
      );
    } else {
      cells.push(null);
    }
    var covTitle = "Line coverage";
    if (group.coverage) {
      covTitle += " (" + group.coverage[0] + "/" + group.coverage[1] + " lines)";
    }
    if (group.coverage_pct !== null && group.coverage_pct !== undefined) {
      cells.push(labeledDonut(group.coverage_pct, "Cov", covTitle, group.codecov_url));
    } else {
      cells.push(null);
    }
    var scen = group.scenarios || {};
    SCENARIO_BADGES.forEach(function (entry) {
      var s = scen[entry[0]];
      if (s === "passed" || s === "failed") {
        var wrap = document.createElement("div");
        wrap.className = "pn-badge-wrap";
        wrap.appendChild(miniStatus(entry[0], entry[1], entry[2], s));
        cells.push(wrap);
      } else {
        cells.push(null);
      }
    });
    if (!cells.some(Boolean)) {
      return null;
    }
    var strip = document.createElement("div");
    strip.className = "pn-badges";
    strip.setAttribute("data-group", stem);
    strip.setAttribute("data-section", section);
    cells.forEach(function (c) {
      var el = document.createElement("div");
      el.className = "pn-cell" + (c ? "" : " pn-empty");
      if (c) {
        el.appendChild(c);
      }
      strip.appendChild(el);
    });
    return strip;
  }

  function rowName(firstCell) {
    if (!firstCell) {
      return null;
    }
    var code = firstCell.querySelector("code.c-func");
    if (code) {
      return code.textContent.replace(/\(\)\s*$/, "").trim();
    }
    var p = firstCell.querySelector("p");
    if (p) {
      var txt = p.textContent.trim();
      if (/^[A-Za-z_]\w*$/.test(txt)) {
        return txt;
      }
    }
    return null;
  }

  /* which badge slots have content for any row of this table/page? */
  function computeSlots(db, scenarios, names) {
    var iso = db.iso_c || {};
    var funcs = db.functions || {};
    var slots = {cov: false, scenarios: [], iso: false, status: false};
    SCENARIO_BADGES.forEach(function (entry) {
      var s = scenarios[entry[0]];
      if (s === "passed" || s === "failed") {
        slots.scenarios.push(entry);
      }
    });
    names.forEach(function (name) {
      if (!name) {
        return;
      }
      var f = funcs[name] || {};
      if (f.coverage_pct !== null && f.coverage_pct !== undefined) {
        slots.cov = true;
      }
      if (Object.prototype.hasOwnProperty.call(iso, name)) {
        slots.iso = true;
      }
      if (f.status === "stub" || f.status === "unsupported") {
        slots.status = true;
      }
    });
    return slots;
  }

  /* Effective per-function status of a scenario class: sanitizers and
     static analysis only report what fails, so when a failed variant has
     failure attribution, a function is red only when implicated. */
  function scenarioStatusFor(name, key, scenarios, scenarioFailed) {
    var status = scenarios[key];
    var failed = (scenarioFailed || {})[key];
    if (status === "failed" && failed !== undefined) {
      return failed.indexOf(name) !== -1 ? "failed" : "passed";
    }
    return status;
  }

  /* slot order: coverage, scenarios..., ISO C, implementation status;
     only slots with content somewhere in the table are reserved */
  function buildFunctionStrip(db, scenarios, scenarioFailed, slots, name) {
    var iso = db.iso_c || {};
    var f = (db.functions || {})[name] || {};
    var strip = document.createElement("span");
    strip.className = "pn-func-badges";
    var any = false;

    if (slots.cov) {
      var donut = null;
      if (f.coverage_pct !== null && f.coverage_pct !== undefined) {
        var title = name + " line coverage";
        if (f.coverage) {
          title += " (" + f.coverage[0] + "/" + f.coverage[1] + " lines)";
        }
        donut = miniDonut(f.coverage_pct, title, f.codecov_url);
        any = true;
      }
      strip.appendChild(cell(donut));
    }

    slots.scenarios.forEach(function (entry) {
      var status = scenarioStatusFor(name, entry[0], scenarios, scenarioFailed);
      any = true;
      strip.appendChild(cell(miniStatus(entry[0], entry[1], entry[2], status)));
    });

    if (slots.iso) {
      var isoBadge = null;
      if (Object.prototype.hasOwnProperty.call(iso, name)) {
        isoBadge = isoCBadge(name, iso[name], isoCStatus(scenarios));
        any = true;
      }
      strip.appendChild(cell(isoBadge));
    }

    if (slots.status) {
      var statusPill = null;
      if (f.status === "stub") {
        statusPill = pill("ENOSYS", "pn-enosys", name + " is an ENOSYS stub (not implemented)");
        any = true;
      } else if (f.status === "unsupported") {
        statusPill = pill("✗", "pn-enosys", name + " is not supported");
        any = true;
      }
      strip.appendChild(cell(statusPill, true));
    }

    return any ? strip : null;
  }

  /* badge strip after the title of an Option Group / Option detail page */
  function decorateDetail(db) {
    var page = detailPage();
    if (!page) {
      return;
    }
    var group = (db[page.section] || {})[page.stem];
    if (!group) {
      return;
    }
    var strip = buildGroupStrip(page.stem, group, page.section);
    var title = document.querySelector('[role="main"] h1');
    if (strip && title && title.parentNode) {
      title.parentNode.insertBefore(strip, title.nextSibling);
    }
  }

  /* drop the redundant Supported column, preserving footnote references */
  function dropSupportedColumn(tbl, rows) {
    var headRow = tbl.querySelector("thead tr");
    var th = headRow && headRow.children[1];
    if (!th || !/supported/i.test(th.textContent)) {
      return;
    }
    th.remove();
    rows.forEach(function (row) {
      var td = row.children[1];
      if (!td) {
        return;
      }
      var target = row.children[0].querySelector("p") || row.children[0];
      td.querySelectorAll("a").forEach(function (a) {
        target.appendChild(document.createTextNode(" "));
        target.appendChild(a);
      });
      td.remove();
    });
  }

  function decorateFunctions(db) {
    var page = detailPage();
    if (!page) {
      return;
    }
    var group = (db[page.section] || {})[page.stem] || {};
    var scenarios = group.scenarios || {};
    var scenarioFailed = group.scenario_failed_functions || {};

    document.querySelectorAll('[role="main"] table').forEach(function (tbl) {
      var rows = Array.prototype.slice.call(tbl.querySelectorAll("tbody tr"));
      if (!rows.length) {
        return;
      }
      var names = rows.map(function (row) {
        return rowName(row.querySelector("td:first-child"));
      });
      /* transform API tables uniformly, even when no row has badge data;
         the gate is the table shape, not badge availability */
      var headRow0 = tbl.querySelector("thead tr");
      var th1 = headRow0 && headRow0.children[1];
      if (!th1 || !/supported/i.test(th1.textContent)) {
        return;
      }
      var slots = computeSlots(db, scenarios, names);
      var strips = names.map(function (name) {
        return name ? buildFunctionStrip(db, scenarios, scenarioFailed, slots, name) : null;
      });

      /* let content size the table instead of the baked-in csv widths */
      tbl.querySelectorAll("colgroup").forEach(function (cg) {
        cg.remove();
      });
      dropSupportedColumn(tbl, rows);

      /* badges get their own real table column so they align vertically */
      var headRow = tbl.querySelector("thead tr");
      if (headRow) {
        var th = document.createElement("th");
        th.className = "head pn-badges-col";
        th.textContent = "Status";
        headRow.appendChild(th);
      }
      rows.forEach(function (row, i) {
        var td = document.createElement("td");
        td.className = "pn-badges-col";
        if (strips[i]) {
          td.appendChild(strips[i]);
        }
        row.appendChild(td);
      });
    });
  }

  /* Doxygen group pages: strip under the page title (Option Group /
     Option pages), per-function strips at the top of each member doc. */
  function decorateDoxygen(db) {
    var page = doxygenPage();
    if (!page) {
      return;
    }
    document.body.classList.add("pn-doxy");
    var group = page.section ? (db[page.section] || {})[page.stem] : null;
    var scenarios = (group && group.scenarios) || {};
    var scenarioFailed = (group && group.scenario_failed_functions) || {};

    if (group) {
      var strip = buildGroupStrip(page.stem, group, page.section);
      var header = document.querySelector("div.header");
      if (strip && header && header.parentNode) {
        header.parentNode.insertBefore(strip, header.nextSibling);
      }
    }

    var members = [];
    document.querySelectorAll("h2.memtitle").forEach(function (h2) {
      var m = h2.textContent.match(/(\w+)\(\)\s*$/);
      var item = h2.nextElementSibling;
      var memdoc = item && item.querySelector ? item.querySelector(".memdoc") : null;
      if (m && memdoc && (db.functions || {})[m[1]]) {
        members.push({name: m[1], memdoc: memdoc});
      }
    });
    if (!members.length) {
      return;
    }
    var slots = computeSlots(db, scenarios, members.map(function (mb) {
      return mb.name;
    }));
    members.forEach(function (mb) {
      var fnStrip = buildFunctionStrip(db, scenarios, scenarioFailed, slots, mb.name);
      if (fnStrip) {
        mb.memdoc.insertBefore(fnStrip, mb.memdoc.firstChild);
      }
    });
  }

  /* Replace the index toctree bullet lists with tables like the detail
     pages: an Option Group link column plus an aligned Status column.
     Each visible toctree on the page becomes its own table; hidden
     toctrees render as empty wrappers and are left alone. */
  function decorateIndex(db) {
    var section = indexSection();
    if (!section) {
      return;
    }
    var groups = db[section] || {};
    document
      .querySelectorAll('[role="main"] .toctree-wrapper')
      .forEach(function (wrapper) {
        decorateToctree(wrapper, groups, section);
      });
  }

  function decorateToctree(wrapper, groups, section) {
    var items = [];
    wrapper
      .querySelectorAll("li.toctree-l1 > a.reference.internal")
      .forEach(function (link) {
        var href = link.getAttribute("href") || "";
        var stem = href.replace(/[?#].*$/, "").replace(/\.html$/, "");
        if (!stem || stem.indexOf("/") !== -1) {
          return;
        }
        items.push({link: link, group: groups[stem] || null});
      });
    if (!items.length) {
      return;
    }

    /* columns only for badge classes any group on the page has */
    var slots = {impl: false, cov: false, scenarios: []};
    var scenarioSeen = {};
    items.forEach(function (item) {
      var g = item.group;
      if (!g) {
        return;
      }
      if (g.completeness_pct !== null && g.completeness_pct !== undefined) {
        slots.impl = true;
      }
      if (g.coverage_pct !== null && g.coverage_pct !== undefined) {
        slots.cov = true;
      }
      var scen = g.scenarios || {};
      SCENARIO_BADGES.forEach(function (entry) {
        var s = scen[entry[0]];
        if (s === "passed" || s === "failed") {
          scenarioSeen[entry[0]] = true;
        }
      });
    });
    /* keep the canonical SCENARIO_BADGES order, not first-seen order */
    SCENARIO_BADGES.forEach(function (entry) {
      if (scenarioSeen[entry[0]]) {
        slots.scenarios.push(entry);
      }
    });
    if (!slots.impl && !slots.cov && !slots.scenarios.length) {
      return;
    }

    function buildIndexStrip(group) {
      var strip = document.createElement("span");
      strip.className = "pn-func-badges";
      var g = group || {};
      if (slots.impl) {
        var impl = null;
        if (g.completeness_pct !== null && g.completeness_pct !== undefined) {
          impl = miniDonut(g.completeness_pct, "Implementation completeness");
        }
        strip.appendChild(cell(impl));
      }
      if (slots.cov) {
        var cov = null;
        if (g.coverage_pct !== null && g.coverage_pct !== undefined) {
          cov = miniDonut(g.coverage_pct, "Line coverage", g.codecov_url);
        }
        strip.appendChild(cell(cov));
      }
      var scen = g.scenarios || {};
      slots.scenarios.forEach(function (entry) {
        var s = scen[entry[0]];
        var badge = null;
        if (s === "passed" || s === "failed") {
          badge = miniStatus(entry[0], entry[1], entry[2], s);
        }
        strip.appendChild(cell(badge));
      });
      return strip;
    }

    var table = document.createElement("table");
    table.className = "docutils align-default pn-index-table";
    var thead = document.createElement("thead");
    var headRow = document.createElement("tr");
    [section === "options" ? "Option" : "Option Group", "Status"].forEach(function (text) {
      var th = document.createElement("th");
      th.className = "head";
      th.textContent = text;
      headRow.appendChild(th);
    });
    thead.appendChild(headRow);
    table.appendChild(thead);

    var tbody = document.createElement("tbody");
    items.forEach(function (item) {
      var tr = document.createElement("tr");
      var tdLink = document.createElement("td");
      tdLink.appendChild(item.link.cloneNode(true));
      tr.appendChild(tdLink);
      var tdBadges = document.createElement("td");
      tdBadges.className = "pn-badges-col";
      if (item.group) {
        tdBadges.appendChild(buildIndexStrip(item.group));
      }
      tr.appendChild(tdBadges);
      tbody.appendChild(tr);
    });
    table.appendChild(tbody);

    /* keep the toctree in the DOM (navigation metadata) but hide it */
    wrapper.style.display = "none";
    wrapper.parentNode.insertBefore(table, wrapper);
  }

  function run() {
    if (!detailPage() && !indexSection() && !doxygenPage()) {
      return;
    }
    fetchDb()
      .then(function (db) {
        if (db.thresholds && db.thresholds.length === 2) {
          thresholds = db.thresholds;
        }
        decorateDetail(db);
        decorateFunctions(db);
        decorateIndex(db);
        decorateDoxygen(db);
      })
      .catch(function (err) {
        console.warn("posix-badges:", err);
      });
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", run);
  } else {
    run();
  }
})();

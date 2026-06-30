/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * AEP profile tabs: hash deep-links and interactive diagram labels.
 */
(function () {
  const LEGACY_HASH_TARGETS = {
    "system-interfaces": "posix-aep-system-interfaces",
    "minimal-realtime-system-profile-pse51": "posix-aep-pse51",
    "realtime-controller-system-profile-pse52": "posix-aep-pse52",
    "dedicated-realtime-system-profile-pse53": "posix-aep-pse53",
  };

  const HASH_TO_TAB_LABEL = {
    "posix-aep-system-interfaces": "System Interfaces",
    "system-interfaces": "System Interfaces",
    "posix-aep-pse51": "PSE51",
    "minimal-realtime-system-profile-pse51": "PSE51",
    "posix-aep-pse52": "PSE52",
    "realtime-controller-system-profile-pse52": "PSE52",
    "posix-aep-pse53": "PSE53",
    "dedicated-realtime-system-profile-pse53": "PSE53",
  };

  const TAB_LABEL_TO_HASH = {
    "System Interfaces": "#posix-aep-system-interfaces",
    PSE51: "#posix-aep-pse51",
    PSE52: "#posix-aep-pse52",
    PSE53: "#posix-aep-pse53",
  };

  function hashSlug() {
    if (!window.location.hash || window.location.hash.length < 2) {
      return null;
    }
    return decodeURIComponent(window.location.hash.slice(1)).toLowerCase();
  }

  function normalizeId(raw) {
    return String(raw).replace(/^#/, "");
  }

  function canonicalHashId(slug) {
    return LEGACY_HASH_TARGETS[slug] || slug;
  }

  function tabLabelForId(rawId) {
    const id = normalizeId(rawId).toLowerCase();
    return HASH_TO_TAB_LABEL[id] || HASH_TO_TAB_LABEL[canonicalHashId(id)];
  }

  function preserveScroll(action) {
    const scrollX = window.scrollX;
    const scrollY = window.scrollY;
    action();
    window.requestAnimationFrame(() => {
      window.requestAnimationFrame(() => {
        window.scrollTo(scrollX, scrollY);
      });
    });
  }

  function activateTabByLabel(label) {
    const tabs = document.querySelectorAll(".sphinx-tabs-tab");
    for (const tab of tabs) {
      if (tab.textContent.trim() === label) {
        if (tab.getAttribute("aria-selected") !== "true") {
          preserveScroll(() => {
            tab.click();
          });
        }
        return true;
      }
    }
    return false;
  }

  function activateTabForTarget(target) {
    const panel = target.closest(".sphinx-tabs-panel");
    if (!panel) {
      return false;
    }

    const tabId = panel.getAttribute("aria-labelledby");
    if (!tabId) {
      return false;
    }

    const tab = document.getElementById(tabId);
    if (!tab) {
      return false;
    }

    if (tab.getAttribute("aria-selected") !== "true") {
      preserveScroll(() => {
        tab.click();
      });
    }

    return true;
  }

  function isAepTabPage() {
    return document.querySelector(".sphinx-tabs") !== null;
  }

  function aepTabHash(rawId) {
    return "#" + normalizeId(rawId);
  }

  function aepTabHref(rawId, container) {
    const hash = aepTabHash(rawId);
    const base = container?.getAttribute("data-aep-href-base");
    if (base) {
      return base + hash;
    }
    if (isAepTabPage()) {
      return hash;
    }
    return new URL("../aep/index.html" + hash, window.location.href).pathname + hash;
  }

  function diagramSvgUrl(container) {
    const src = container.getAttribute("data-aep-svg");
    return new URL(src, window.location.href).href;
  }

  function activateTabQuietly(rawId) {
    if (!isAepTabPage()) {
      return false;
    }

    const id = normalizeId(rawId);
    const target = document.getElementById(id);
    if (target && activateTabForTarget(target)) {
      return true;
    }

    const label = tabLabelForId(rawId);
    if (label) {
      return activateTabByLabel(label);
    }

    return false;
  }

  function updateHashWithoutScroll(hash) {
    if (window.location.hash === hash) {
      return;
    }

    const url = window.location.pathname + window.location.search + hash;
    history.replaceState(null, "", url);
  }

  function switchAepTab(rawId, options) {
    const opts = options || {};
    const id = normalizeId(rawId);
    const hash = "#" + id;

    if (isAepTabPage()) {
      activateTabQuietly(rawId);
      if (opts.updateHash !== false) {
        updateHashWithoutScroll(hash);
      }
      return false;
    }

    window.location.href = new URL("../aep/index.html" + hash, window.location.href).href;
    return false;
  }

  function syncTabFromHash() {
    const slug = hashSlug();
    if (!slug) {
      return;
    }

    const id = canonicalHashId(slug);
    const target = document.getElementById(id);
    if (target && activateTabForTarget(target)) {
      window.requestAnimationFrame(() => {
        target.scrollIntoView({ block: "start" });
      });
      return;
    }

    const label = HASH_TO_TAB_LABEL[slug] || HASH_TO_TAB_LABEL[id];
    if (label) {
      activateTabByLabel(label);
    }
  }

  function aepDiagramHover(rawId) {
    if (!isAepTabPage()) {
      return;
    }
    const hash = "#" + normalizeId(rawId);
    activateTabQuietly(rawId);
    updateHashWithoutScroll(hash);
  }

  function aepDiagramSelect(rawId) {
    return switchAepTab(rawId, { updateHash: true });
  }

  function setDiagramLinkHref(el, tabId, container) {
    const href = aepTabHref(tabId, container);
    el.setAttributeNS("http://www.w3.org/1999/xlink", "href", href);
    el.setAttribute("href", href);
  }

  function wireAepDiagramLink(el, container) {
    const tabId = el.getAttribute("data-aep-tab");
    if (!tabId) {
      return;
    }

    setDiagramLinkHref(el, tabId, container);

    el.addEventListener("click", (event) => {
      event.preventDefault();
      switchAepTab(tabId, { updateHash: true });
    });
    el.addEventListener("mouseenter", () => {
      aepDiagramHover(tabId);
    });
  }

  function disableClosableAepTabs() {
    document.querySelectorAll("button.sphinx-tabs-tab, a.sphinx-tabs-tab").forEach((tab) => {
      const label = tab.textContent.trim();
      if (TAB_LABEL_TO_HASH[label] && tab.parentNode) {
        tab.parentNode.classList.remove("closeable");
      }
    });
  }

  function upgradeTabButtonsToLinks() {
    if (!isAepTabPage()) {
      return;
    }

    disableClosableAepTabs();

    document.querySelectorAll("button.sphinx-tabs-tab").forEach((button) => {
      const label = button.textContent.trim();
      const hash = TAB_LABEL_TO_HASH[label];
      if (!hash) {
        return;
      }

      const link = document.createElement("a");
      link.className = button.className;
      for (const attr of button.attributes) {
        if (attr.name !== "type") {
          link.setAttribute(attr.name, attr.value);
        }
      }
      link.href = hash;
      link.textContent = button.textContent;

      link.addEventListener(
        "click",
        (event) => {
          event.preventDefault();
          updateHashWithoutScroll(hash);
          if (link.getAttribute("aria-selected") === "true") {
            event.stopImmediatePropagation();
            event.stopPropagation();
          }
        },
        true
      );
      link.addEventListener("mouseenter", () => {
        aepDiagramHover(hash.slice(1));
      });

      button.replaceWith(link);
    });
  }

  async function inlineAepDiagram(container) {
    const src = container.getAttribute("data-aep-svg");
    if (!src || container.dataset.aepInline === "1") {
      return;
    }

    const url = diagramSvgUrl(container);
    const response = await fetch(url);
    if (!response.ok) {
      return;
    }

    const holder = document.createElement("div");
    holder.className = "aep-diagram-svg";
    holder.innerHTML = await response.text();

    const svg = holder.querySelector("svg");
    if (!svg) {
      return;
    }

    svg.removeAttribute("width");
    svg.removeAttribute("height");
    svg.setAttribute("aria-hidden", "true");

    holder.querySelectorAll("[data-aep-tab]").forEach((el) => {
      wireAepDiagramLink(el, container);
    });

    const fallback = container.querySelector(".aep-diagram-fallback");
    if (fallback) {
      fallback.remove();
    }

    const caption = container.querySelector(".caption");
    container.insertBefore(holder, caption);
    container.dataset.aepInline = "1";
  }

  function prepareAepDiagrams() {
    document.querySelectorAll(".aep-diagram[data-aep-svg]").forEach((container) => {
      inlineAepDiagram(container);
    });
  }

  window.switchAepTab = switchAepTab;
  window.aepDiagramHover = aepDiagramHover;
  window.aepDiagramSelect = aepDiagramSelect;

  document.addEventListener("DOMContentLoaded", () => {
    upgradeTabButtonsToLinks();
    syncTabFromHash();
    prepareAepDiagrams();
  });
  window.addEventListener("hashchange", syncTabFromHash);
})();

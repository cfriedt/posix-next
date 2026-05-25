/*
 * Copyright (c) The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 *
 * POSIX API Symbol Search
 * Structurally equivalent to Zephyr's kconfig.mjs.
 */

// Resolve relative to this module's own URL (_static/posix-symbols.mjs),
// so the path is correct regardless of which page embeds it.
const DB_FILE = new URL('posix-symbols.json', import.meta.url).href;
const RESULTS_PER_PAGE_OPTIONS = [10, 25, 50];

/* search state */
let db;
let doxyHtmlUrl;
let searchOffset;
let maxResults = RESULTS_PER_PAGE_OPTIONS[0];

/* elements */
let input;
let searchTools;
let summaryText;
let results;
let navigation;
let navigationPagesText;
let navigationPrev;
let navigationNext;

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

/** Kind → badge CSS class */
const KIND_CLASS = {
    'function': 'kind-fn',
    'type':     'kind-type',
    'macro':    'kind-macro',
    'struct':   'kind-struct',
    'union':    'kind-union',
    'enum':     'kind-enum',
    'variable': 'kind-var',
};

/**
 * Build a Doxygen HTML anchor URL for a symbol refid.
 * refids look like "group__posix__threads_1a3c..." — the part before '_1a'
 * is the group file, and the whole id is the anchor.
 */
function doxyUrl(refid) {
    if (!doxyHtmlUrl || !refid) return null;
    const sep = refid.lastIndexOf('_1');
    if (sep < 0) return null;
    const file = refid.substring(0, sep) + '.html';
    return `${doxyHtmlUrl}/${file}#${refid}`;
}

function showError(message) {
    const admonition = document.createElement('div');
    admonition.className = 'admonition error';
    results.replaceChildren(admonition);

    const title = document.createElement('p');
    title.className = 'admonition-title';
    title.textContent = 'Error';
    admonition.appendChild(title);

    const body = document.createElement('p');
    body.textContent = message;
    admonition.appendChild(body);
}

function showProgress(message) {
    const p = document.createElement('p');
    p.className = 'centered';
    p.textContent = message;
    results.replaceChildren(p);
}

/* -------------------------------------------------------------------------
 * Render a single symbol entry (mirrors renderKconfigEntry)
 * ---------------------------------------------------------------------- */

function renderSymbolEntry(entry) {
    const container = document.createElement('dl');
    container.className = 'posix-symbol';
    container.id = entry.name;

    /* ---- title row: name + kind badge + permalink ---- */
    const dt = document.createElement('dt');
    dt.className = 'sig sig-object';
    container.appendChild(dt);

    /* Symbol name — linked to Doxygen if we have a refid */
    const url = doxyUrl(entry.doxy_refid);
    let nameEl;
    if (url) {
        nameEl = document.createElement('a');
        nameEl.href = url;
        nameEl.className = 'symbol-name';
    } else {
        nameEl = document.createElement('span');
        nameEl.className = 'symbol-name';
    }
    nameEl.textContent = entry.name;
    dt.appendChild(nameEl);

    /* Kind badge */
    const badge = document.createElement('span');
    badge.className = `symbol-kind ${KIND_CLASS[entry.kind] || ''}`;
    badge.textContent = entry.kind;
    dt.appendChild(badge);

    /* Permalink anchor */
    const permalink = document.createElement('a');
    permalink.className = 'headerlink';
    permalink.href = '#' + entry.name;
    permalink.textContent = '\u00b6';   /* ¶ */
    dt.appendChild(permalink);

    /* ---- details ---- */
    const dd = document.createElement('dd');
    container.appendChild(dd);

    /* Brief description */
    if (entry.brief) {
        const brief = document.createElement('p');
        brief.textContent = entry.brief;
        dd.appendChild(brief);
    }

    /* Metadata table */
    const props = document.createElement('dl');
    props.className = 'field-list simple';
    dd.appendChild(props);

    function addProp(label, valueEl) {
        const term = document.createElement('dt');
        term.textContent = label;
        props.appendChild(term);
        const details = document.createElement('dd');
        details.appendChild(valueEl);
        props.appendChild(details);
    }

    function textProp(label, text) {
        const code = document.createElement('code');
        code.className = 'docutils literal';
        const span = document.createElement('span');
        span.className = 'pre';
        span.textContent = text;
        code.appendChild(span);
        addProp(label, code);
    }

    /* Header file */
    if (entry.header) {
        textProp('Header', entry.header);
    }

    /* POSIX Option Group */
    if (entry.group_label) {
        const groupEl = document.createElement('span');
        groupEl.textContent = entry.group_label;
        addProp('Option group', groupEl);
    }

    /* Open Group specification link */
    if (entry.opengroup_url) {
        const link = document.createElement('a');
        link.href = entry.opengroup_url;
        link.target = '_blank';
        link.rel = 'noopener noreferrer';
        link.textContent = 'POSIX.1-2017 specification ↗';
        addProp('Standard', link);
    }

    return container;
}

/* -------------------------------------------------------------------------
 * Search engine (same scoring as kconfig.mjs)
 * ---------------------------------------------------------------------- */

function doSearch() {
    history.replaceState(
        { value: input.value, searchOffset },
        '',
        window.location
    );

    if (!input.value) {
        results.replaceChildren();
        navigation.style.visibility = 'hidden';
        searchTools.style.visibility = 'hidden';
        return;
    }

    const regexes = input.value.trim().split(/\s+/).map(
        token => new RegExp(token.toLowerCase())
    );

    const scored = db.map(entry => {
        const name = entry.name.toLowerCase();
        const brief = entry.brief ? entry.brief.toLowerCase() : '';
        const header = entry.header ? entry.header.toLowerCase() : '';

        let nameMatches = 0;
        let briefMatches = 0;

        regexes.forEach(re => {
            if (re.test(name))   nameMatches++;
            if (re.test(brief))  briefMatches++;
        });

        const total = Math.max(nameMatches, briefMatches);
        if (total < regexes.length) return null;

        const NAME_W  = 2.5;
        const BRIEF_W = 1.0;
        const score =
            (nameMatches  * NAME_W  * (1 / Math.sqrt(name.length)))  +
            (briefMatches * BRIEF_W * (brief ? 1 / Math.sqrt(brief.length) : 0));

        return { entry, score };
    }).filter(Boolean).sort((a, b) => b.score - a.score);

    const count = scored.length;
    const page = scored
        .slice(searchOffset, searchOffset + maxResults)
        .map(r => r.entry);

    summaryText.nodeValue = `${count} symbol${count !== 1 ? 's' : ''} match your search.`;
    searchTools.style.visibility = 'visible';

    navigation.style.visibility = 'visible';
    navigationPrev.disabled = searchOffset - maxResults < 0;
    navigationNext.disabled = searchOffset + maxResults >= count;

    const currentPage = Math.floor(searchOffset / maxResults) + 1;
    const totalPages  = Math.max(1, Math.ceil(count / maxResults));
    navigationPagesText.nodeValue = `Page ${currentPage} of ${totalPages}`;

    results.replaceChildren();
    page.forEach(entry => results.appendChild(renderSymbolEntry(entry)));
}

function doSearchFromURL() {
    const raw = window.location.hash.substring(1);
    if (!raw) return;
    const option = decodeURIComponent(raw);
    input.value = option.startsWith('!') ? option.substring(1) : `^${option}$`;
    searchOffset = 0;
    doSearch();
}

/* -------------------------------------------------------------------------
 * Initialisation
 * ---------------------------------------------------------------------- */

function setupPosixSearch() {
    const container = document.getElementById('__posix-search');
    if (!container) {
        console.error("posix-symbols: couldn't find #__posix-search");
        return;
    }

    /* --- input row --- */
    const inputContainer = document.createElement('div');
    inputContainer.className = 'input-container';
    container.appendChild(inputContainer);

    input = document.createElement('input');
    input.type = 'text';
    input.placeholder = 'Type a POSIX symbol name (RegEx allowed)';
    inputContainer.appendChild(input);

    const copyBtn = document.createElement('button');
    copyBtn.title = 'Copy link to results';
    copyBtn.textContent = '🔗';
    copyBtn.onclick = () => {
        if (!window.isSecureContext) return;
        const url = `${location.protocol}//${location.host}${location.pathname}#!${input.value}`;
        navigator.clipboard.writeText(encodeURI(url));
    };
    inputContainer.appendChild(copyBtn);

    /* --- search tools bar --- */
    searchTools = document.createElement('div');
    searchTools.className = 'search-tools';
    searchTools.style.visibility = 'hidden';
    container.appendChild(searchTools);

    const summaryEl = document.createElement('p');
    summaryText = document.createTextNode('');
    summaryEl.appendChild(summaryText);
    searchTools.appendChild(summaryEl);

    const rppContainer = document.createElement('div');
    rppContainer.className = 'results-per-page-container';
    searchTools.appendChild(rppContainer);

    const rppTitle = document.createElement('span');
    rppTitle.className = 'results-per-page-title';
    rppTitle.textContent = 'Results per page:';
    rppContainer.appendChild(rppTitle);

    const rppSelect = document.createElement('select');
    rppSelect.onchange = e => {
        maxResults = parseInt(e.target.value);
        searchOffset = 0;
        doSearch();
    };
    RESULTS_PER_PAGE_OPTIONS.forEach((v, i) => {
        const opt = document.createElement('option');
        opt.value = v;
        opt.text = v;
        opt.selected = i === 0;
        rppSelect.appendChild(opt);
    });
    rppContainer.appendChild(rppSelect);

    /* --- results area --- */
    results = document.createElement('div');
    container.appendChild(results);

    /* --- pagination --- */
    navigation = document.createElement('div');
    navigation.className = 'search-nav';
    navigation.style.visibility = 'hidden';
    container.appendChild(navigation);

    navigationPrev = document.createElement('button');
    navigationPrev.className = 'btn';
    navigationPrev.disabled = true;
    navigationPrev.textContent = 'Previous';
    navigationPrev.onclick = () => {
        searchOffset -= maxResults;
        doSearch();
        window.scroll(0, 0);
    };
    navigation.appendChild(navigationPrev);

    const pagesEl = document.createElement('p');
    navigation.appendChild(pagesEl);
    navigationPagesText = document.createTextNode('');
    pagesEl.appendChild(navigationPagesText);

    navigationNext = document.createElement('button');
    navigationNext.className = 'btn';
    navigationNext.disabled = true;
    navigationNext.textContent = 'Next';
    navigationNext.onclick = () => {
        searchOffset += maxResults;
        doSearch();
        window.scroll(0, 0);
    };
    navigation.appendChild(navigationNext);

    /* --- load database --- */
    showProgress('Loading symbol database...');

    fetch(DB_FILE)
        .then(r => r.json())
        .then(json => {
            db = json.symbols;
            doxyHtmlUrl = (json.doxy_html_url || '').replace(/\/$/, '');
            searchOffset = 0;
            results.replaceChildren();

            doSearchFromURL();

            input.addEventListener('input', () => {
                searchOffset = 0;
                doSearch();
            });

            window.addEventListener('hashchange', doSearchFromURL);

            window.addEventListener('popstate', e => {
                if (!e.state) return;
                input.value = e.state.value;
                searchOffset = e.state.searchOffset;
                doSearch();
            });
        })
        .catch(err => showError(`POSIX symbol database could not be loaded (${err})`));
}

setupPosixSearch();

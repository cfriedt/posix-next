/*
 * Ensure Doxygen symbol hover tooltips have an opaque background.
 * Styles are injected into the doxytooltip shadow root because the tooltip
 * content lives outside the main document stylesheet scope.
 */
(function () {
  const STYLE = `
.tippy-box[data-theme~='doxytooltip'] {
  background-color: var(--content-wrap-background-color, #fcfcfc) !important;
  color: var(--body-color, #404040) !important;
}
.tippy-box[data-theme~='doxytooltip'][data-placement^='top'] > .tippy-arrow::before {
  border-top-color: var(--content-wrap-background-color, #fcfcfc) !important;
}
.tippy-box[data-theme~='doxytooltip'][data-placement^='bottom'] > .tippy-arrow::before {
  border-bottom-color: var(--content-wrap-background-color, #fcfcfc) !important;
}
.tippy-box[data-theme~='doxytooltip'][data-placement^='left'] > .tippy-arrow::before {
  border-left-color: var(--content-wrap-background-color, #fcfcfc) !important;
}
.tippy-box[data-theme~='doxytooltip'][data-placement^='right'] > .tippy-arrow::before {
  border-right-color: var(--content-wrap-background-color, #fcfcfc) !important;
}`;

  function applyPatch() {
    const host = document.getElementById('doxytooltip');
    if (!host?.shadowRoot || host.shadowRoot.getElementById('posix-doxytooltip-patch')) {
      return;
    }
    const style = document.createElement('style');
    style.id = 'posix-doxytooltip-patch';
    style.textContent = STYLE;
    host.shadowRoot.appendChild(style);
  }

  document.addEventListener('DOMContentLoaded', () => {
    applyPatch();
    new MutationObserver(applyPatch).observe(document.body, { childList: true, subtree: true });
  });
})();

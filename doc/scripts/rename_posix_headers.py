#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Post-process Doxygen HTML to rename posix_<name>.h → <name>.h in display text.

The physical header files carry a posix_ prefix to avoid colliding with
standard C headers of the same name, but the POSIX API presented to users
should show the standard names (e.g. <stdlib.h> rather than posix_stdlib.h).

This script rewrites the link text and page titles in the generated Doxygen
HTML while keeping all href attributes (which use the mangled physical name)
intact.
"""

import pathlib
import re
import sys

RENAME_MAP = {
    "posix_limits": "limits",
    "posix_signal": "signal",
    "posix_stdlib": "stdlib",
    "posix_string": "string",
    "posix_time": "time",
}


def rewrite_file(path: pathlib.Path) -> bool:
    text = path.read_text(encoding="utf-8")
    original = text

    for old_stem, new_stem in RENAME_MAP.items():
        # Link text: >posix_limits.h</a>  →  >limits.h</a>
        text = text.replace(f">{old_stem}.h</a>", f">{new_stem}.h</a>")
        # Page titles in <div class="title">
        text = text.replace(
            f"{old_stem}.h File Reference", f"{new_stem}.h File Reference"
        )
        text = text.replace(f"{old_stem}.h Source File", f"{new_stem}.h Source File")
        # Breadcrumb / tree text that is NOT inside an href
        text = re.sub(
            rf"(<span[^>]*>)\s*{re.escape(old_stem)}\.h\s*(</span>)",
            rf"\g<1>{new_stem}.h\2",
            text,
        )

    if text != original:
        path.write_text(text, encoding="utf-8")
        return True
    return False


def main() -> None:
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <doxygen-html-dir>", file=sys.stderr)
        sys.exit(1)

    html_dir = pathlib.Path(sys.argv[1])
    if not html_dir.is_dir():
        print(f"Error: {html_dir} is not a directory", file=sys.stderr)
        sys.exit(1)

    count = 0
    for html_file in sorted(html_dir.glob("*.html")):
        if rewrite_file(html_file):
            count += 1

    print(f"rename_posix_headers: rewrote {count} file(s) in {html_dir}")


if __name__ == "__main__":
    main()

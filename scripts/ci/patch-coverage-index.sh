#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Sync gcovr index.html summary totals with merged.json. gcovr --html-details
# can show incorrect directory/file rollups when rendering merged trace JSON
# (--merge-mode-functions=separate); the header summary is authoritative for CI.

set -euo pipefail

usage() {
  echo "Usage: $0 MERGED_JSON INDEX_HTML" >&2
  exit 2
}

merged_json="${1:?MERGED_JSON required}"
index_html="${2:?INDEX_HTML required}"

if [ ! -f "$merged_json" ] || [ ! -f "$index_html" ]; then
  echo "patch-coverage-index: missing input file" >&2
  exit 1
fi

command -v python3 >/dev/null 2>&1 || {
  echo "python3 is required" >&2
  exit 1
}

python3 - "$merged_json" "$index_html" <<'PY'
import json
import re
import sys
from pathlib import Path

merged_path = Path(sys.argv[1])
index_path = Path(sys.argv[2])
data = json.loads(merged_path.read_text())

line_hit = line_total = 0
func_hit = func_total = func_excl = 0
branch_hit = branch_total = branch_excl = 0

for entry in data.get("files", []):
    lines = entry.get("lines") or []
    line_total += len(lines)
    line_hit += sum(1 for ln in lines if (ln.get("count") or 0) > 0)

    funcs = entry.get("functions") or []
    for fn in funcs:
        if fn.get("ignored"):
            func_excl += 1
            continue
        func_total += 1
        if (fn.get("execution_count") or 0) > 0:
            func_hit += 1

    branches = entry.get("branches") or []
    for br in branches:
        if br.get("ignored"):
            branch_excl += 1
            continue
        branch_total += 1
        if (br.get("count") or 0) > 0:
            branch_hit += 1

def pct(hit: int, total: int) -> str:
    if total <= 0:
        return "-"
    return f"{hit * 100 / total:.1f}"

def css_class(value: str) -> str:
    if value == "-":
        return "coverage-unknown"
    num = float(value)
    if num >= 90.0:
        return "coverage-high"
    if num >= 75.0:
        return "coverage-medium"
    return "coverage-low"

rows = {
    "Lines": (pct(line_hit, line_total), f"{line_hit} / 0 / {line_total}"),
    "Functions": (pct(func_hit, func_total), f"{func_hit} / {func_excl} / {func_total + func_excl}"),
    "Branches": (pct(branch_hit, branch_total), f"{branch_hit} / {branch_excl} / {branch_total + branch_excl}"),
}

html = index_path.read_text()
for label, (percent, counts) in rows.items():
    if label == "Branches" and branch_total + branch_excl == 0:
        continue
    css = css_class(percent)
    display = f"{percent}%" if percent != "-" else "-%"
    replacement = (
        f'<tr>\n        <th scope="row">{label}:</th>\n'
        f'        <td class="{css}">{display}</td>\n'
        f'        <td class="{css}">{counts}</td>\n'
        f"      </tr>"
    )
    pattern = rf"<tr>\s*<th scope=\"row\">{re.escape(label)}:</th>.*?</tr>"
    new_html, n = re.subn(pattern, replacement, html, count=1, flags=re.S)
    if n != 1:
        print(f"patch-coverage-index: failed to patch {label} row", file=sys.stderr)
        sys.exit(1)
    html = new_html

index_path.write_text(html)
print(
    f"patch-coverage-index: Lines {rows['Lines'][0]}%, "
    f"Functions {rows['Functions'][0]}%, Branches {rows['Branches'][0]}%"
)
PY

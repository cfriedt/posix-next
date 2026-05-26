#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Print the manifest repo path relative to the workspace root.
set -euo pipefail

base_path="${1:?base-path required}"
app_path="${2:?app-path required}"
manifest_file="${3:-west.yml}"

cd "$base_path"

west_yml="${app_path}/${manifest_file}"
if [ ! -f "$west_yml" ]; then
  echo "west manifest not found: $west_yml" >&2
  exit 1
fi

python3 - "$west_yml" "$app_path" <<'PY'
import sys
from pathlib import Path

import yaml

west_yml = Path(sys.argv[1])
fallback = sys.argv[2].strip("./").rstrip("/") or "."

data = yaml.safe_load(west_yml.read_text()) or {}
self_path = (data.get("manifest") or {}).get("self", {}).get("path")
if self_path:
    print(str(self_path).strip("./").rstrip("/"))
else:
    print(fallback)
PY

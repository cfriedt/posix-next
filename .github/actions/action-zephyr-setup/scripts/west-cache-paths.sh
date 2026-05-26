#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# List west project paths to cache (manifest repo excluded).
set -euo pipefail

base_path="${1:?base-path required}"
app_path="${2:?app-path required}"

cd "$base_path"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
manifest_norm="$("$script_dir/west-read-manifest-path.sh" "$base_path" "$app_path")"

{
  echo ".west"
  west list -f '{path}' | while IFS= read -r path; do
    path="${path#./}"
    path="${path%/}"
    if [ -z "$path" ] || [ "$path" = "$manifest_norm" ]; then
      continue
    fi
    echo "$path"
  done
} | sort -u

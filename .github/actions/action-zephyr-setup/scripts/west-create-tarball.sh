#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Archive west projects (and .west) into a zstd tarball; manifest repo excluded.
set -euo pipefail

base_path="${1:?base-path required}"
app_path="${2:?app-path required}"
tarball="${3:?tarball output path required}"

cd "$base_path"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mapfile -t paths < <("$script_dir/west-cache-paths.sh" . "$app_path")

if [ "${#paths[@]}" -eq 0 ]; then
  echo "no west project paths to archive" >&2
  exit 1
fi

mkdir -p "$(dirname "$tarball")"
tar -cf - "${paths[@]}" | zstd -T0 -q -o "$tarball"
echo "Created $(du -h "$tarball" | cut -f1) archive with ${#paths[@]} top-level paths"

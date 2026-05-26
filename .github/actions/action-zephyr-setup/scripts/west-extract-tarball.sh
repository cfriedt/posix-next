#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Extract a west workspace tarball into the workspace root.
set -euo pipefail

base_path="${1:?base-path required}"
tarball="${2:?tarball path required}"

cd "$base_path"

if [ ! -f "$tarball" ]; then
  echo "tarball not found: $tarball" >&2
  exit 1
fi

zstd -d -q -c "$tarball" | tar -xf -
echo "Extracted workspace from $tarball"

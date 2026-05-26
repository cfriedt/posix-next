#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Initialize west so the workspace topdir is base-path and the manifest lives at
# manifest-path (including nested layouts like modules/lib/posix with manifest.self.path).
set -euo pipefail

base_path="${1:?base-path required}"
app_path="${2:?app-path required}"
manifest_file="${3:-west.yml}"

cd "$base_path"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
manifest_path="$("$script_dir/west-read-manifest-path.sh" "$base_path" "$app_path" "$manifest_file")"

if [ ! -d "$manifest_path" ]; then
  echo "manifest directory missing: $base_path/$manifest_path" >&2
  exit 1
fi

west_topdir() {
  west topdir 2>/dev/null || true
}

write_local_config() {
  mkdir -p .west
  cat > .west/config <<EOF
[manifest]
path = ${manifest_path}
file = ${manifest_file}
EOF
}

current_topdir="$(west_topdir)"
if [ -n "$current_topdir" ] && [ "$(realpath "$current_topdir")" != "$(realpath "$PWD")" ]; then
  echo "west already initialized in $current_topdir (expected $(pwd))" >&2
  exit 1
fi

if [ -f .west/config ]; then
  configured="$(west config manifest.path 2>/dev/null || true)"
  if [ "$configured" = "$manifest_path" ]; then
    echo "west workspace already configured (manifest.path=$manifest_path)"
    exit 0
  fi
  echo "Reconfiguring west manifest.path ($configured -> $manifest_path)"
  west config manifest.path "$manifest_path"
  west config manifest.file "$manifest_file"
  exit 0
fi

# west init -l only places .west at the workspace root when the manifest path is a
# single top-level directory name. Nested manifest paths (manifest.self.path) need a
# local .west/config instead.
case "$manifest_path" in
  */*)
    echo "Using local .west/config for nested manifest path: $manifest_path"
    write_local_config
    ;;
  *)
    echo "Running west init -l $manifest_path"
    west init -l "$manifest_path" --mf "$manifest_file"
    ;;
esac

configured="$(west config manifest.path 2>/dev/null || true)"
if [ "$configured" != "$manifest_path" ]; then
  west config manifest.path "$manifest_path"
fi

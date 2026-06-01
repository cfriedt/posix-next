#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
# Place the checked-out posix manifest at manifest-path (e.g. modules/lib/posix).
set -euo pipefail

base_path="${1:?base-path required}"
manifest_path="${2:?manifest-path required}"

cd "$base_path"

if [ -f "${manifest_path}/west.yml" ]; then
  echo "manifest already at ${manifest_path}/west.yml"
  exit 0
fi

if [ ! -f "west.yml" ]; then
  echo "no west.yml at workspace root or ${manifest_path}/" >&2
  exit 1
fi

echo "Re-homing manifest from workspace root to ${manifest_path}/"
mkdir -p "$manifest_path"

shopt -s dotglob nullglob
for entry in *; do
  case "$entry" in
    # Do not skip manifest zephyr/ (module.yml, patches). Re-home runs before west
    # update, so root zephyr/ is never the Zephyr RTOS tree here.
    modules|zephyr-sdk|.west|.ccache)
      continue
      ;;
    .github)
      cp -aR .github "$manifest_path"/.github
      continue
      ;;
  esac
  mv "$entry" "$manifest_path/"
done
shopt -u dotglob nullglob

if [ ! -f "${manifest_path}/west.yml" ]; then
  echo "re-home failed: ${manifest_path}/west.yml missing" >&2
  exit 1
fi

if [ ! -f "${manifest_path}/zephyr/module.yml" ]; then
  echo "re-home failed: ${manifest_path}/zephyr/module.yml missing (manifest Zephyr module metadata)" >&2
  exit 1
fi

echo "Manifest re-homed to ${manifest_path}/"

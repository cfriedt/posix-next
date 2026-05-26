#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Regenerate per-test coverage.json using CMAKE_GCOV from each build's
# CMakeCache.txt. Twister defaults to x86_64-zephyr-elf-gcov for cross-target
# builds; this matches what CMake selected for the build.

set -euo pipefail

usage() {
  echo "Usage: $0 TWISTER_OUT [WORKSPACE_ROOT]" >&2
  exit 2
}

twister_out="${1:?TWISTER_OUT required}"
workspace_root="${2:-$(realpath "$(dirname "$twister_out")")}"

SCRIPT_PATH="$(realpath "$(dirname "$0")")"
POSIX_NEXT_PATH="$(realpath "$SCRIPT_PATH/../..")"
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"

# shellcheck source=gcovr-config-args.sh
source "${SCRIPT_PATH}/gcovr-config-args.sh"

workspace_filter_args=()
if [ -f "$CI_CONFIG" ]; then
  gcovr_load_filter_args workspace_filter_args workspace "$CI_CONFIG"
fi

command -v gcovr >/dev/null 2>&1 || {
  echo "gcovr is required" >&2
  exit 1
}

gcovr_version=$(gcovr --version | head -1 | sed 's/gcovr //')
gcovr_ge_7=false
gcovr_ge_8=false
if printf '%s\n%s\n' "7.0" "$gcovr_version" | sort -CV 2>/dev/null; then
  gcovr_ge_7=true
fi
if printf '%s\n%s\n' "8.0" "$gcovr_version" | sort -CV 2>/dev/null; then
  gcovr_ge_8=true
fi

branch_excludes=(
  --exclude-branches-by-pattern
  '(^\s*LOG_(?:HEXDUMP_)?(?:DBG|INF|WRN|ERR)\(.*)|(^\s*__ASSERT(?:_EVAL|_NO_MSG|_POST_ACTION)?\(.*)'
)

is_native_build_dir() {
  local build_dir="$1"
  [[ "$build_dir" == *native_sim* || "$build_dir" == */host/* ]]
}

refreshed=0
skipped=0
no_gcda=0

while IFS= read -r cache; do
  build_dir=$(dirname "$cache")
  kconfig="$build_dir/zephyr/.config"
  if [ ! -f "$kconfig" ] || ! grep -q '^CONFIG_COVERAGE=y' "$kconfig" 2>/dev/null; then
    continue
  fi

  rel="${build_dir#"$twister_out"/}"
  gcda_count=$(find "$build_dir" -name '*.gcda' 2>/dev/null | wc -l)
  if [ "$gcda_count" -eq 0 ]; then
    echo "refresh-coverage: no .gcda in ${rel}" >&2
    no_gcda=$((no_gcda + 1))
    skipped=$((skipped + 1))
    continue
  fi

  gcov_tool=$(sed -n 's/^CMAKE_GCOV:FILEPATH=//p' "$cache" | head -1)
  if is_native_build_dir "$build_dir"; then
    gcov_tool=$(command -v gcov || true)
  fi
  if [ -z "$gcov_tool" ] || [ ! -x "$gcov_tool" ]; then
    echo "refresh-coverage: no gcov tool for ${rel} (CMAKE_GCOV missing or not executable)" >&2
    skipped=$((skipped + 1))
    continue
  fi

  coverage_json="$build_dir/coverage.json"
  covlog="$build_dir/coverage.log"
  : > "$covlog"
  object_dir_args=()
  if [ "$gcovr_ge_7" = true ]; then
    object_dir_args=(--gcov-object-directory "$build_dir")
  fi
  ignore_args=()
  if [ "$gcovr_ge_8" = true ]; then
    ignore_args=(--gcov-ignore-parse-errors=suspicious_hits.warn_once_per_file)
  fi

  if ! gcovr -r "$workspace_root" \
      --gcov-ignore-parse-errors=negative_hits.warn_once_per_file \
      --gcov-executable "$gcov_tool" \
      -e 'tests/*' \
      -e '.*generated.*' \
      -e '.*/tests/.*' \
      -e '.*/samples/.*' \
      "${ignore_args[@]}" \
      "${object_dir_args[@]}" \
      --merge-mode-functions=separate \
      "${branch_excludes[@]}" \
      "${workspace_filter_args[@]}" \
      --json -o "$coverage_json" \
      "$build_dir" >> "$covlog" 2>&1; then
    echo "refresh-coverage: gcovr failed for ${rel} (${gcda_count} .gcda)" >&2
    if [ -s "$covlog" ]; then
      tail -3 "$covlog" >&2 || true
    fi
    skipped=$((skipped + 1))
    continue
  fi

  refreshed=$((refreshed + 1))
done < <(find "$twister_out" -name CMakeCache.txt -type f ! -path "$twister_out/CMakeCache.txt")

echo "refresh-coverage: regenerated ${refreshed} trace(s), skipped ${skipped} (${no_gcda} without .gcda)"

#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Regenerate per-test coverage.json using CMAKE_GCOV from each build's
# CMakeCache.txt. Twister defaults to x86_64-zephyr-elf-gcov for cross-target
# builds; this matches what CMake selected for the build.
#
# Parallelism: REFRESH_JOBS (default: nproc). Skip native rebuild when Twister
# already produced a non-empty trace. Skip entirely with
# COVERAGE_SKIP_REFRESH=1.

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
REFRESH_JOBS="${REFRESH_JOBS:-$(nproc)}"

# shellcheck source=gcovr-config-args.sh
source "${SCRIPT_PATH}/gcovr-config-args.sh"

workspace_filter_args=()
if [ -f "$CI_CONFIG" ]; then
  gcovr_load_filter_args workspace_filter_args workspace "$CI_CONFIG"
fi

command -v gcovr jq >/dev/null 2>&1 || {
  echo "gcovr and jq are required" >&2
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

branch_exclude_pattern='(^\s*LOG_(?:HEXDUMP_)?(?:DBG|INF|WRN|ERR)\(.*)|'\
'(^\s*__ASSERT(?:_EVAL|_NO_MSG|_POST_ACTION)?\(.*)'
branch_excludes=(
  --exclude-branches-by-pattern
  "$branch_exclude_pattern"
)

is_native_build_dir() {
  local build_dir="$1"
  [[ "$build_dir" == *native_sim* || "$build_dir" == */host/* ]]
}

trace_has_hits() {
  local trace="$1"
  local hits

  hits=$(jq -r '[(.files[]?.lines[]?.count // empty) | select(. > 0)] | length' \
    "$trace" 2>/dev/null || echo 0)
  [ -f "$trace" ] && [ "$hits" -gt 0 ]
}

refresh_build_dir() {
  local cache="$1"
  local build_dir gcov_tool coverage_json covlog rel gcda_count
  local -a object_dir_args ignore_args

  build_dir=$(dirname "$cache")
  kconfig="$build_dir/zephyr/.config"
  if [ ! -f "$kconfig" ] || \
     ! grep -q '^CONFIG_COVERAGE=y' "$kconfig" 2>/dev/null; then
    echo "skip:no_coverage_config"
    return 0
  fi

  rel="${build_dir#"$twister_out"/}"
  coverage_json="$build_dir/coverage.json"

  if is_native_build_dir "$build_dir" && trace_has_hits "$coverage_json"; then
    echo "skip:existing_native_trace"
    return 0
  fi

  gcda_count=$(find "$build_dir" -name '*.gcda' 2>/dev/null | wc -l)
  if [ "$gcda_count" -eq 0 ]; then
    echo "skip:no_gcda"
    return 0
  fi

  gcov_tool=$(sed -n 's/^CMAKE_GCOV:FILEPATH=//p' "$cache" | head -1)
  if is_native_build_dir "$build_dir"; then
    gcov_tool=$(command -v gcov || true)
  fi
  if [ -z "$gcov_tool" ] || [ ! -x "$gcov_tool" ]; then
    echo "skip:no_gcov_tool"
    return 0
  fi

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

  if gcovr -r "$workspace_root" \
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
    echo "ok:${rel}"
    return 0
  fi

  echo "fail:${rel}" >&2
  if [ -s "$covlog" ]; then
    tail -3 "$covlog" >&2 || true
  fi
  return 1
}

export -f refresh_build_dir trace_has_hits is_native_build_dir
export twister_out workspace_root CI_CONFIG gcovr_ge_7 gcovr_ge_8
export branch_exclude_pattern branch_excludes workspace_filter_args

mapfile -t caches < <(
  find "$twister_out" -name CMakeCache.txt -type f ! -path "$twister_out/CMakeCache.txt"
)

if [ ${#caches[@]} -eq 0 ]; then
  echo "refresh-coverage: no build directories under ${twister_out}"
  exit 0
fi

results_file=$(mktemp "${TMPDIR:-/tmp}/refresh-coverage.XXXXXX")
trap 'rm -f "$results_file"' EXIT

printf '%s\0' "${caches[@]}" | \
  xargs -0 -P "$REFRESH_JOBS" -I {} bash -c 'refresh_build_dir "$1" || true' _ {} \
  > "$results_file"

refreshed=$(grep -c '^ok:' "$results_file" || true)
skipped_native=$(grep -c '^skip:existing_native_trace' "$results_file" || true)
skipped_no_gcda=$(grep -c '^skip:no_gcda' "$results_file" || true)
skipped_other=$(grep -c '^skip:' "$results_file" || true)
failed=$(grep -c '^fail:' "$results_file" || true)

echo "refresh-coverage: regenerated ${refreshed} trace(s), skipped ${skipped_other} "\
  "(${skipped_native} native reuse, ${skipped_no_gcda} without .gcda), failed ${failed} "\
  "(jobs=${REFRESH_JOBS})"

#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -euo pipefail

usage() {
  echo "Usage: $0 [--non-empty-out FILE] TWISTER_OUT_DIR [coverage.json ...]" >&2
  echo "  With no files: scan twister-out for per-test coverage.json traces." >&2
  echo "  Prints COVERAGE_EMPTY / COVERAGE_TRACE_SUMMARY lines for CI logs." >&2
  exit 2
}

non_empty_out=""

while [ $# -gt 0 ]; do
  case "$1" in
    --non-empty-out)
      non_empty_out="${2:?}"
      shift 2
      ;;
    -h|--help)
      usage
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage
      ;;
    *)
      break
      ;;
  esac
done

twister_out="${1:-}"
shift || true

if [ -z "$twister_out" ]; then
  usage
fi

command -v jq >/dev/null 2>&1 || {
  echo "jq is required" >&2
  exit 1
}

declare -a coverage_files=()
if [ $# -gt 0 ]; then
  coverage_files=("$@")
else
  shopt -s nullglob
  mapfile -t coverage_files < <(
    find "$twister_out" -name coverage.json -type f \
      ! -path "$twister_out/coverage-full.json" \
      ! -path "$twister_out/coverage-posix.json"
  )
fi

diagnose_build_dir() {
  local build_dir="$1"
  local handler="$build_dir/handler.log"
  local covlog="$build_dir/coverage.log"
  local kconfig="$build_dir/zephyr/.config"
  local reason=""
  local detail=""

  if [ -f "$kconfig" ] && ! grep -q '^CONFIG_COVERAGE=y' "$kconfig" 2>/dev/null; then
    echo "coverage_not_enabled|CONFIG_COVERAGE is not set in zephyr/.config (check -p / --coverage-platform)"
    return
  fi

  if [ ! -f "$handler" ]; then
    echo "missing_handler_log|no handler.log in build directory"
    return
  fi

  if ! grep -q 'GCOV_COVERAGE_DUMP_START' "$handler" 2>/dev/null; then
    shopt -s nullglob
    local gcda_count
    gcda_count=$(find "$build_dir" -name '*.gcda' 2>/dev/null | wc -l)
    if [ "$gcda_count" -gt 0 ]; then
      if [ -f "$covlog" ] && grep -qi 'All coverage data is filtered out' "$covlog" 2>/dev/null; then
        echo "gcovr_all_filtered|native/host .gcda present but gcovr filtered all data"
      else
        echo "gcovr_empty_with_gcda|.gcda files exist but coverage.json is empty; see coverage.log"
      fi
      return
    fi
    if grep -qiE 'FATAL ERROR|Stack overflow|ASSERT|ZEPHYR FATAL' "$handler" 2>/dev/null; then
      echo "no_gcov_dump_test_failed|handler.log has no GCOV dump (test may have crashed before dump)"
    else
      echo "no_gcov_dump|handler.log has no GCOV_COVERAGE_DUMP_START (serial dump missing on HW; native may lack CONFIG_COVERAGE_NATIVE_GCOV)"
    fi
    return
  fi

  if ! grep -q 'GCOV_COVERAGE_DUMP_END' "$handler" 2>/dev/null; then
    echo "incomplete_gcov_dump|GCOV_COVERAGE_DUMP_START present but END missing"
    return
  fi

  shopt -s nullglob
  local gcda_count
  gcda_count=$(find "$build_dir" -name '*.gcda' 2>/dev/null | wc -l)
  if [ "$gcda_count" -eq 0 ]; then
    echo "no_gcda_files|gcov dump markers found but no .gcda under build dir after capture"
    return
  fi

  if [ -f "$covlog" ]; then
    if grep -qi 'All coverage data is filtered out' "$covlog" 2>/dev/null; then
      echo "gcovr_all_filtered|gcovr reported all coverage data filtered (check -r and -e excludes)"
      return
    fi
    detail=$(grep -iE '^\(ERROR\)|^\(WARNING\)|GCOVR failed' "$covlog" 2>/dev/null | tail -1 | sed 's/^[[:space:]]*//')
    if [ -n "$detail" ]; then
      echo "gcovr_warning|${detail}"
      return
    fi
  fi

  echo "empty_trace_unknown|coverage.json has files:[] but dump and .gcda exist; see coverage.log"
}

declare -A reason_counts=()
non_empty=0
empty=0
declare -a non_empty_files=()

for trace in "${coverage_files[@]}"; do
  if [ ! -f "$trace" ]; then
    continue
  fi

  file_count=$(jq -r '.files | length' "$trace" 2>/dev/null || echo 0)
  hit_count=$(jq -r '
    [(.files[]?.lines[]?.count // empty),
     (.files[]?.functions[]?.execution_count // empty)]
    | map(select(. != null and . > 0)) | length
  ' "$trace" 2>/dev/null || echo 0)
  rel="${trace#"$twister_out"/}"
  build_dir="$(dirname "$trace")"

  if [ "$file_count" -gt 0 ] && [ "$hit_count" -gt 0 ]; then
    non_empty=$((non_empty + 1))
    non_empty_files+=("$trace")
    continue
  fi

  empty=$((empty + 1))
  if [ "$file_count" -gt 0 ] && [ "$hit_count" -eq 0 ]; then
    reason="zero_hit_trace"
    detail="coverage.json lists instrumented lines/functions but every hit count is 0"
  else
    IFS='|' read -r reason detail < <(diagnose_build_dir "$build_dir")
  fi
  reason_counts["$reason"]=$((${reason_counts[$reason]:-0} + 1))
  echo "COVERAGE_EMPTY ${rel}: ${reason} — ${detail}"
done

summary="COVERAGE_TRACE_SUMMARY: ${non_empty} non-empty, ${empty} empty"
if [ "$empty" -gt 0 ]; then
  parts=()
  for reason in "${!reason_counts[@]}"; do
    parts+=("${reason}=${reason_counts[$reason]}")
  done
  summary+=", empty breakdown: $(IFS=,; echo "${parts[*]}")"
fi
echo "$summary"

if [ -n "$non_empty_out" ]; then
  : > "$non_empty_out"
  if [ ${#non_empty_files[@]} -gt 0 ]; then
    printf '%s\n' "${non_empty_files[@]}" > "$non_empty_out"
  fi
fi

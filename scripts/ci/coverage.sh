#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

REALPATH="realpath"
SCRIPT_PATH="$(realpath "$(dirname "$0")")"

# Default local run (same platforms as coverage_nightly, POSIX-only roots):
#   ./scripts/ci/coverage.sh
#
# Full nightly scope:
#   CI_CONFIG_PROFILE=coverage_nightly ./scripts/ci/coverage.sh
#
# Skip Twister (merge/view only, requires existing twister-out):
#   COVERAGE_SKIP_TWISTER=1 ./scripts/ci/coverage.sh
#
# Skip re-running gcovr on every build dir when Twister traces look good:
#   COVERAGE_SKIP_REFRESH=1 ./scripts/ci/coverage.sh
#
# Other slow postprocess steps (compose as needed):
#   COVERAGE_SKIP_SLIM=1       skip jq slimming of per-test traces
#   COVERAGE_SKIP_DIAGNOSE=1   skip per-trace diagnosis (uses a fast find fallback)
#   COVERAGE_SKIP_MERGE=1      skip gcovr merge (requires twister-out/coverage-full.json)
#   COVERAGE_SKIP_FILTER=1     skip posix filter (requires twister-out/coverage-posix.json)
#   COVERAGE_SKIP_UI=1         skip launching coverageui.py locally
#   COVERAGE_SKIP_ALL=1        skip every postprocess step except merge/filter/ui
#
# Re-open coverageui.py against an existing coverage-posix.json:
#   COVERAGE_UI_ONLY=1 ./scripts/ci/coverage.sh
#
# Prerequisites (once):
#   source ~/posix-next/zephyr/zephyr-env.sh
#   pip install gcovr jq
#   pip install -r modules/lib/posix/scripts/ci/requirements-coverage.txt

export CI_CONFIG_PROFILE="${CI_CONFIG_PROFILE:-coverage_local}"

coverage_wants_skip() {
  local step="$1"
  local var="COVERAGE_SKIP_${step}"
  if [ "${COVERAGE_SKIP_ALL:-0}" = "1" ] && [ "$step" != "UI" ]; then
    return 0
  fi
  [ "${!var:-0}" = "1" ]
}

if [ "${COVERAGE_UI_ONLY:-0}" = "1" ]; then
  export COVERAGE_SKIP_TWISTER=1
  export COVERAGE_SKIP_REFRESH=1
  export COVERAGE_SKIP_SLIM=1
  export COVERAGE_SKIP_DIAGNOSE=1
  export COVERAGE_SKIP_MERGE=1
  export COVERAGE_SKIP_FILTER=1
fi

collect_merge_traces() {
  local out="$1"
  local twister_out="$2"

  : > "$out"
  mapfile -t traces < <(
    find "$twister_out" -name coverage.json -type f \
      ! -path "$twister_out/coverage-full.json" \
      ! -path "$twister_out/coverage-posix.json"
  )

  for trace in "${traces[@]}"; do
    if [ ! -s "$trace" ]; then
      continue
    fi
    workspace_trace="${trace%.json}.workspace.json"
    if [ -s "$workspace_trace" ]; then
      printf '%s\n' "$workspace_trace" >> "$out"
    else
      printf '%s\n' "$trace" >> "$out"
    fi
  done
}

twister_rc=0
if coverage_wants_skip TWISTER || [ "${COVERAGE_POSTPROCESS_ONLY:-0}" = "1" ]; then
  echo "Skipping Twister (COVERAGE_SKIP_TWISTER=1)"
else
  "$SCRIPT_PATH"/runci.sh "$@" || twister_rc=$?
fi

WORKSPACE="$(west topdir)"
CI_CONFIG="$WORKSPACE/modules/lib/posix/.github/ci-config.json"
SCRIPTS="$WORKSPACE/modules/lib/posix/scripts/ci"

cd "$WORKSPACE"

if coverage_wants_skip REFRESH; then
  echo "Skipping refresh-coverage-traces (COVERAGE_SKIP_REFRESH=1)"
else
  echo "Refreshing coverage trace(s)"
  "$SCRIPTS/refresh-coverage-traces.sh" twister-out "$WORKSPACE"
fi

if coverage_wants_skip SLIM; then
  echo "Skipping slim-coverage-traces (COVERAGE_SKIP_SLIM=1)"
else
  echo "Slimming coverage trace(s)"
  "$SCRIPTS/slim-coverage-traces.sh" twister-out "$CI_CONFIG"
fi

non_empty_traces="${TMPDIR:-/tmp}/non-empty-coverage-traces.txt"
if coverage_wants_skip DIAGNOSE; then
  echo "Skipping diagnose-coverage-trace (COVERAGE_SKIP_DIAGNOSE=1)"
  collect_merge_traces "$non_empty_traces" "$WORKSPACE/twister-out"
else
  echo "Diagnosing coverage trace(s)"
  diagnose="$SCRIPTS/diagnose-coverage-trace.sh"
  "$diagnose" --non-empty-out "$non_empty_traces" --prefer-workspace twister-out
fi

if coverage_wants_skip MERGE && coverage_wants_skip FILTER; then
  if [ ! -s "$WORKSPACE/twister-out/coverage-posix.json" ]; then
    echo "coverage-posix.json missing; cannot skip merge and filter" >&2
    exit 1
  fi
  echo "Skipping merge and filter (using existing twister-out/coverage-posix.json)"
elif coverage_wants_skip MERGE; then
  if [ ! -s "$WORKSPACE/twister-out/coverage-full.json" ]; then
    echo "coverage-full.json missing; cannot skip merge" >&2
    exit 1
  fi
  echo "Skipping gcovr merge (COVERAGE_SKIP_MERGE=1)"
  echo "Generating coverage-posix.json"
  filter="$SCRIPTS/filter-coverage-json.sh"
  "$filter" --scope posix --ci-config "$CI_CONFIG" \
    "$WORKSPACE/twister-out/coverage-full.json" \
    "$WORKSPACE/twister-out/coverage-posix.json"
else
  if [ ! -s "$non_empty_traces" ]; then
    echo "No non-empty coverage traces; skipping gcovr merge" >&2
    exit 0
  fi

  mapfile -t TRACES < "$non_empty_traces"
  merge="$SCRIPTS/merge-coverage-json.sh"
  filter="$SCRIPTS/filter-coverage-json.sh"

  mkdir -p twister-out

  echo "Generating coverage-full.json from ${#TRACES[@]} trace(s)"
  "$merge" --workspace "$WORKSPACE" \
    --output "$WORKSPACE/twister-out/coverage-full.json" \
    --ci-config "$CI_CONFIG" \
    --filter-scope workspace \
    -- "${TRACES[@]}"

  if coverage_wants_skip FILTER; then
    if [ ! -s "$WORKSPACE/twister-out/coverage-posix.json" ]; then
      echo "coverage-posix.json missing; cannot skip filter" >&2
      exit 1
    fi
    echo "Skipping posix filter (COVERAGE_SKIP_FILTER=1)"
  else
    echo "Generating coverage-posix.json"
    "$filter" --scope posix --ci-config "$CI_CONFIG" \
      "$WORKSPACE/twister-out/coverage-full.json" \
      "$WORKSPACE/twister-out/coverage-posix.json"
  fi
fi

if [ "${GITHUB_ACTIONS:-}" != "true" ] && ! coverage_wants_skip UI; then
  python3 "$SCRIPTS/coverageui.py" \
    -d "$WORKSPACE" \
    --framework posix \
    "$WORKSPACE/twister-out/coverage-posix.json"
elif coverage_wants_skip UI; then
  echo "Skipping coverageui (COVERAGE_SKIP_UI=1)"
fi

if [ "$twister_rc" -ne 0 ]; then
  echo "Twister finished with failures (exit ${twister_rc});" \
       "coverage reports were still generated." >&2
fi

exit "$twister_rc"

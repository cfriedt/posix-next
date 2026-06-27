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
# Prerequisites (once):
#   source ~/posix-next/zephyr/zephyr-env.sh
#   pip install gcovr jq
#   pip install -r modules/lib/posix/scripts/ci/requirements-coverage.txt

export CI_CONFIG_PROFILE="${CI_CONFIG_PROFILE:-coverage_local}"

twister_rc=0
if [ "${COVERAGE_SKIP_TWISTER:-${COVERAGE_POSTPROCESS_ONLY:-0}}" = "1" ]; then
  echo "Skipping Twister (COVERAGE_SKIP_TWISTER=1)"
else
  "$SCRIPT_PATH"/runci.sh "$@" || twister_rc=$?
fi

WORKSPACE="$(west topdir)"
CI_CONFIG="$WORKSPACE/modules/lib/posix/.github/ci-config.json"
SCRIPTS="$WORKSPACE/modules/lib/posix/scripts/ci"

cd "$WORKSPACE"

echo "Refreshing coverage trace(s)"

if [ "${COVERAGE_SKIP_REFRESH:-0}" != "1" ]; then
  "$SCRIPTS/refresh-coverage-traces.sh" twister-out "$WORKSPACE"
else
  echo "Skipping refresh-coverage-traces (COVERAGE_SKIP_REFRESH=1)"
fi

echo "Slimming coverage trace(s)"

"$SCRIPTS/slim-coverage-traces.sh" twister-out "$CI_CONFIG"

echo "Diagnosing coverage trace(s)"

diagnose="$SCRIPTS/diagnose-coverage-trace.sh"
non_empty_traces="${TMPDIR:-/tmp}/non-empty-coverage-traces.txt"
"$diagnose" --non-empty-out "$non_empty_traces" --prefer-workspace twister-out

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

echo "Generating coverage-posix.json"

"$filter" --scope posix --ci-config "$CI_CONFIG" \
  "$WORKSPACE/twister-out/coverage-full.json" \
  "$WORKSPACE/twister-out/coverage-posix.json"

if [ "${GITHUB_ACTIONS:-}" != "true" ]; then
  python3 "$SCRIPTS/coverageui.py" \
    -d "$WORKSPACE" \
    --framework posix \
    "$WORKSPACE/twister-out/coverage-posix.json"
fi

if [ "$twister_rc" -ne 0 ]; then
  echo "Twister finished with failures (exit ${twister_rc});" \
       "coverage reports were still generated." >&2
fi

exit "$twister_rc"

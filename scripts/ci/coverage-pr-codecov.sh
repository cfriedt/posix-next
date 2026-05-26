#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Run native_sim coverage on an existing PR testplan and merge gcovr JSON.

set -euo pipefail

TESTPLAN="${1:?testplan.json required}"

SCRIPT_PATH="$(realpath "$(dirname "$0")")"
POSIX_NEXT_PATH="$(realpath "$SCRIPT_PATH/../..")"
WORKSPACE_PATH="$(realpath "$POSIX_NEXT_PATH/../../..")"
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"

if [[ "$TESTPLAN" != /* ]]; then
  TESTPLAN="$WORKSPACE_PATH/$TESTPLAN"
fi
if [ ! -f "$TESTPLAN" ] || [ "$(jq '.testsuites | length' "$TESTPLAN")" -eq 0 ]; then
  echo "No tests in PR test plan; skipping coverage."
  exit 0
fi

export CI_CONFIG_PROFILE=coverage_pr

cd "$WORKSPACE_PATH"
twister_rc=0
"$POSIX_NEXT_PATH/scripts/ci/coverage.sh" \
  -i \
  --load-tests "$TESTPLAN" \
  || twister_rc=$?

"$POSIX_NEXT_PATH/scripts/ci/refresh-coverage-traces.sh" \
  twister-out "$WORKSPACE_PATH"

diagnose="$POSIX_NEXT_PATH/scripts/ci/diagnose-coverage-trace.sh"
non_empty_traces="${RUNNER_TEMP:-/tmp}/non-empty-coverage-traces.txt"
"$diagnose" --non-empty-out "$non_empty_traces" twister-out

if [ -s "$non_empty_traces" ]; then
  gcovr_args=()
  while IFS= read -r a; do gcovr_args+=("$a"); done \
    < <(jq -r '.coverage_report.gcovr_args[]? // empty' "$CI_CONFIG")
  trace_args=()
  while IFS= read -r f; do trace_args+=(--add-tracefile "$f"); done \
    < "$non_empty_traces"
  # shellcheck source=gcovr-config-args.sh
  source "$POSIX_NEXT_PATH/scripts/ci/gcovr-config-args.sh"
  posix_filter_args=()
  gcovr_load_filter_args posix_filter_args posix "$CI_CONFIG"
  gcovr -r "$WORKSPACE_PATH" \
    "${gcovr_args[@]}" \
    "${trace_args[@]}" \
    "${posix_filter_args[@]}" \
    --json twister-out/coverage.json
  line_hits=$(jq '[.files[]?.lines[]?.count // empty | select(. > 0)] | length' twister-out/coverage.json)
  echo "PR coverage merge: ${line_hits} line hits"
else
  echo "PR coverage: no non-empty traces (Codecov upload may be skipped)" >&2
fi

exit "$twister_rc"

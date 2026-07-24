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
  mapfile -t traces < "$non_empty_traces"
  # merge through merge-coverage-json.sh, never raw gcovr --add-tracefile:
  # it strips the function "pos" fields that make gcovr 8.6 crash when
  # host and SDK traces are merged
  "$POSIX_NEXT_PATH/scripts/ci/merge-coverage-json.sh" \
    --workspace "$WORKSPACE_PATH" \
    --output "$WORKSPACE_PATH/twister-out/coverage.json" \
    --ci-config "$CI_CONFIG" \
    --filter-scope posix \
    -- "${traces[@]}"
  gcovr_args=()
  while IFS= read -r a; do gcovr_args+=("$a"); done \
    < <(jq -r '.coverage_report.gcovr_args[]? // empty' "$CI_CONFIG")
  gcovr -r "$WORKSPACE_PATH" \
    "${gcovr_args[@]}" \
    --add-tracefile twister-out/coverage.json \
    --xml-pretty -o twister-out/coverage.xml
  line_hits=$(jq '[.files[]?.lines[]?.count // empty | select(. > 0)] | length' twister-out/coverage.json)
  echo "PR coverage merge: ${line_hits} line hits"
else
  echo "PR coverage: no non-empty traces (Codecov upload may be skipped)" >&2
fi

exit "$twister_rc"

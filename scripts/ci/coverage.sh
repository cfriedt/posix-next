#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

REALPATH="realpath"
SCRIPT_PATH="$(realpath "$(dirname "$0")")"

export CI_CONFIG_PROFILE=coverage_nightly

exec "$SCRIPT_PATH"/runci.sh "$@"

# Note: none of the following commands are executed because they follow an exec call, but it is
# convenient to copy-paste after a coverage run (same merge steps as CI coverage.yml).
#
# Prerequisites (once):
#   source ~/posix-next/zephyr/zephyr-env.sh
#   pip install gcovr jq
#   pip install -r modules/lib/posix/scripts/ci/requirements-coverage.txt
#
# (a) Generate coverage data from tests (runs twister with CI_CONFIG_PROFILE=coverage_nightly):
#   ./modules/lib/posix/scripts/ci/coverage.sh
#
# (b–d) From the west workspace root, refresh traces, merge JSON, and start the UI:

WORKSPACE="$(west topdir)"
CI_CONFIG="$WORKSPACE/modules/lib/posix/.github/ci-config.json"

cd "$WORKSPACE"

"$WORKSPACE/modules/lib/posix/scripts/ci/refresh-coverage-traces.sh" \
  twister-out "$WORKSPACE"

mapfile -t TRACES < <(
  find twister-out -name coverage.json -type f \
    ! -path twister-out/coverage-full.json \
    ! -path twister-out/coverage-posix.json
)

# shellcheck source=gcovr-config-args.sh
source "$WORKSPACE/modules/lib/posix/scripts/ci/gcovr-config-args.sh"

gcovr_args=()
while IFS= read -r a; do gcovr_args+=("$a"); done \
  < <(jq -r '.coverage_report.gcovr_args[]? // empty' "$CI_CONFIG")

workspace_filter_args=()
gcovr_load_filter_args workspace_filter_args workspace "$CI_CONFIG"

posix_filter_args=()
gcovr_load_filter_args posix_filter_args posix "$CI_CONFIG"

trace_args=()
for trace in "${TRACES[@]}"; do
  trace_args+=(--add-tracefile "$trace")
done

mkdir -p twister-out

# (b) coverage-full.json — zephyr + modules/lib/posix (CI workspace filters)
gcovr -r "$WORKSPACE" \
  "${gcovr_args[@]}" \
  "${trace_args[@]}" \
  "${workspace_filter_args[@]}" \
  --json "$WORKSPACE/twister-out/coverage-full.json"

# (c) coverage-posix.json — POSIX headers + lib/posix only (re-filter full merge)
gcovr -r "$WORKSPACE" \
  "${gcovr_args[@]}" \
  --add-tracefile "$WORKSPACE/twister-out/coverage-full.json" \
  "${posix_filter_args[@]}" \
  --json "$WORKSPACE/twister-out/coverage-posix.json"

# (d) Interactive coverage viewer (default http://localhost:8000)
python3 "$WORKSPACE/modules/lib/posix/scripts/ci/coverageui.py" \
  -d "$WORKSPACE" \
  --framework posix \
  "$WORKSPACE/twister-out/coverage-posix.json"

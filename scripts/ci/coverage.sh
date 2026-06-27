#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

REALPATH="realpath"
SCRIPT_PATH="$(realpath "$(dirname "$0")")"

export CI_CONFIG_PROFILE=coverage_nightly

"$SCRIPT_PATH"/runci.sh "$@"
# Prerequisites (once):
#   source ~/posix-next/zephyr/zephyr-env.sh
#   pip install gcovr jq
#   pip install -r modules/lib/posix/scripts/ci/requirements-coverage.txt

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

echo "Generating coverage-full.json"

# (b) coverage-full.json — zephyr + modules/lib/posix (CI workspace filters)
gcovr -r "$WORKSPACE" \
  "${gcovr_args[@]}" \
  "${trace_args[@]}" \
  "${workspace_filter_args[@]}" \
  --json "$WORKSPACE/twister-out/coverage-full.json"

echo "Generating coverage-posix.json"

# (c) coverage-posix.json — POSIX headers + lib/posix only (re-filter full merge)
gcovr -r "$WORKSPACE" \
  "${gcovr_args[@]}" \
  --add-tracefile "$WORKSPACE/twister-out/coverage-full.json" \
  "${posix_filter_args[@]}" \
  --json "$WORKSPACE/twister-out/coverage-posix.json"

# (d) Interactive coverage viewer (default http://localhost:8000; skip in CI)
if [ "${GITHUB_ACTIONS:-}" != "true" ]; then
  python3 "$WORKSPACE/modules/lib/posix/scripts/ci/coverageui.py" \
    -d "$WORKSPACE" \
    --framework posix \
    "$WORKSPACE/twister-out/coverage-posix.json"
fi

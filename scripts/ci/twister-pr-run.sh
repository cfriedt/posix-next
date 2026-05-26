#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Run the twister_pr subset on an existing testplan.json.

set -euo pipefail

SCRIPT_PATH="$(realpath "$(dirname "$0")")"
POSIX_NEXT_PATH="$(realpath "$SCRIPT_PATH/../..")"
WORKSPACE_PATH="$(realpath "$POSIX_NEXT_PATH/../../..")"
ZEPHYR_BASE="$WORKSPACE_PATH/zephyr"
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"

command -v jq >/dev/null 2>&1 || {
  echo "jq is required to read $CI_CONFIG" >&2
  exit 1
}

TESTPLAN="$WORKSPACE_PATH/testplan.json"
if [ ! -f "$TESTPLAN" ] || [ "$(jq '.testsuites | length' "$TESTPLAN")" -eq 0 ]; then
  echo "No tests in test plan; skipping twister."
  exit 0
fi

platforms=()
roots=()
twister_args=()
while IFS= read -r p; do platforms+=(-p "$p"); done \
  < <(jq -r '.twister_pr.platforms[]' "$CI_CONFIG")
while IFS= read -r r; do roots+=(-T "$WORKSPACE_PATH/$r"); done \
  < <(jq -r '.twister_pr.roots[]' "$CI_CONFIG")
while IFS= read -r a; do twister_args+=("$a"); done \
  < <(jq -r '.twister_pr.args[]? // empty' "$CI_CONFIG")

subset_denom=$(jq -r '.twister_pr.subset_denominator // 25' "$CI_CONFIG")

cd "$WORKSPACE_PATH"
exec "$ZEPHYR_BASE/scripts/twister" -c \
  -i \
  --subset "1/${subset_denom}" \
  --load-tests testplan.json \
  "${platforms[@]}" \
  "${roots[@]}" \
  "${twister_args[@]}"

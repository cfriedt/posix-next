#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Build the pull-request test plan (twister_pr profile) from changed_files.json.

set -euo pipefail

CHANGED_FILES="${1:-changed_files.json}"

SCRIPT_PATH="$(realpath "$(dirname "$0")")"
POSIX_NEXT_PATH="$(realpath "$SCRIPT_PATH/../..")"
WORKSPACE_PATH="$(realpath "$POSIX_NEXT_PATH/../../..")"
ZEPHYR_BASE="$WORKSPACE_PATH/zephyr"
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"

command -v jq >/dev/null 2>&1 || {
  echo "jq is required to read $CI_CONFIG" >&2
  exit 1
}

tags_rel=$(jq -r '.paths.tags // ".github/tags.yaml"' "$CI_CONFIG")
ignore_rel=$(jq -r '.paths.twister_ignore // ".github/twister_ignore.txt"' "$CI_CONFIG")
TAGS_CONFIG="$POSIX_NEXT_PATH/$tags_rel"
TWISTER_IGNORE="$POSIX_NEXT_PATH/$ignore_rel"

platforms=()
roots=()
while IFS= read -r p; do platforms+=(-p "$p"); done \
  < <(jq -r '.twister_pr.platforms[]' "$CI_CONFIG")
while IFS= read -r r; do roots+=(-T "$WORKSPACE_PATH/$r"); done \
  < <(jq -r '.twister_pr.roots[]' "$CI_CONFIG")

cd "$WORKSPACE_PATH"
"$ZEPHYR_BASE/scripts/ci/test_plan.py" -r "$POSIX_NEXT_PATH" \
  -m "$CHANGED_FILES" \
  --pull-request \
  -o testplan.json \
  --alt-tags "$TAGS_CONFIG" \
  --ignore-path "$TWISTER_IGNORE" \
  "${platforms[@]}" \
  "${roots[@]}"

test -s testplan.json

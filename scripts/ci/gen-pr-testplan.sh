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

roots=()
while IFS= read -r r; do roots+=(-T "$WORKSPACE_PATH/$r"); done \
  < <(jq -r '.twister_pr.roots[]' "$CI_CONFIG")

# one planning run per platform group (see plan-groups.sh), merged afterwards
mapfile -t groups < <("$SCRIPT_PATH/plan-groups.sh" "$CI_CONFIG" twister_pr)

cd "$WORKSPACE_PATH"
plans=()
i=0
for group in "${groups[@]}"; do
  read -r -a group_platforms <<< "$group"
  platforms=()
  for p in "${group_platforms[@]}"; do platforms+=(-p "$p"); done
  rm -f "testplan.$i.json"
  "$ZEPHYR_BASE/scripts/ci/test_plan.py" -r "$POSIX_NEXT_PATH" \
    -m "$CHANGED_FILES" \
    --pull-request \
    -o "testplan.$i.json" \
    --alt-tags "$TAGS_CONFIG" \
    --ignore-path "$TWISTER_IGNORE" \
    "${platforms[@]}" \
    "${roots[@]}"
  plans+=("testplan.$i.json")
  i=$((i + 1))
done
"$SCRIPT_PATH/merge-testplans.sh" testplan.json "${plans[@]}"
rm -f "${plans[@]}"

count=$(jq '.testsuites | length' testplan.json)
echo "PR test plan: ${count} test(s)"
if [ "$count" -eq 0 ]; then
  echo "No twister tests selected for these changes."
fi

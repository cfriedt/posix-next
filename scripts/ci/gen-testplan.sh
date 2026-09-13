#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -euo pipefail

SCRIPT_PATH="$(realpath "$(dirname "$0")")"
POSIX_NEXT_PATH="$(realpath "$SCRIPT_PATH/../..")"
WORKSPACE_PATH="$(realpath "$POSIX_NEXT_PATH/../../..")"
ZEPHYR_BASE="$WORKSPACE_PATH/zephyr"
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"
PROFILE="${CI_CONFIG_PROFILE:?CI_CONFIG_PROFILE is required}"

command -v jq >/dev/null 2>&1 || {
  echo "jq is required to read $CI_CONFIG" >&2
  exit 1
}

args=()
while IFS= read -r r; do args+=(-T "$WORKSPACE_PATH/$r"); done \
  < <(jq -r --arg profile "$PROFILE" '.[$profile].roots[]' "$CI_CONFIG")
while IFS= read -r a; do args+=("$a"); done \
  < <(jq -r --arg profile "$PROFILE" '.[$profile].plan_args[]? // empty' "$CI_CONFIG")

# one planning run per platform group (see plan-groups.sh), merged afterwards
mapfile -t groups < <("$SCRIPT_PATH/plan-groups.sh" "$CI_CONFIG" "$PROFILE")

cd "$WORKSPACE_PATH"
plans=()
i=0
for group in "${groups[@]}"; do
  read -r -a group_platforms <<< "$group"
  platforms=()
  for p in "${group_platforms[@]}"; do platforms+=(-p "$p"); done
  "$ZEPHYR_BASE/scripts/twister" -c "${platforms[@]}" "${args[@]}" \
    --save-tests "testplan.$i.json" "$@"
  plans+=("testplan.$i.json")
  i=$((i + 1))
done
"$SCRIPT_PATH/merge-testplans.sh" testplan.json "${plans[@]}"
rm -f "${plans[@]}"

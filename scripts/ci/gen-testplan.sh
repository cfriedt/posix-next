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
while IFS= read -r p; do args+=(-p "$p"); done \
  < <(jq -r --arg profile "$PROFILE" '.[$profile].platforms[]' "$CI_CONFIG")
while IFS= read -r r; do args+=(-T "$WORKSPACE_PATH/$r"); done \
  < <(jq -r --arg profile "$PROFILE" '.[$profile].roots[]' "$CI_CONFIG")
while IFS= read -r a; do args+=("$a"); done \
  < <(jq -r --arg profile "$PROFILE" '.[$profile].plan_args[]? // empty' "$CI_CONFIG")

cd "$WORKSPACE_PATH"
exec "$ZEPHYR_BASE/scripts/twister" -c "${args[@]}" --save-tests testplan.json "$@"

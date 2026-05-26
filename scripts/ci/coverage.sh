#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

REALPATH="realpath"
SCRIPT_PATH="$(realpath "$(dirname "$0")")"

export CI_CONFIG_PROFILE=coverage_nightly

exec "$SCRIPT_PATH"/runci.sh "$@"

# Note: none of the following commands are executed because they follow an exec call, but it is
# convenient to copy-paste.

WORKSPACE="$(west topdir)"
CI_CONFIG="$WORKSPACE/modules/lib/posix/.github/ci-config.json"

cd "$WORKSPACE"

mapfile -t TRACES < <(find twister-out -name coverage.json \
  ! -path twister-out/coverage-full.json \
  ! -path twister-out/coverage-posix.json)
# shellcheck source=gcovr-config-args.sh
source "$WORKSPACE/modules/lib/posix/scripts/ci/gcovr-config-args.sh"
posix_filter_args=()
gcovr_load_filter_args posix_filter_args posix "$CI_CONFIG"
mapfile -t GCOVR_ARGS < <(jq -r '.coverage_report.gcovr_args[]? // empty' "$CI_CONFIG")

mkdir -p "$WORKSPACE/twister-out/coverage-posix"

gcovr -r "$WORKSPACE" "${GCOVR_ARGS[@]}" \
  $(printf -- '--add-tracefile %s ' "${TRACES[@]}") \
  $(printf -- '%s ' "${posix_filter_args[@]}") \
  --html-details "$WORKSPACE/twister-out/coverage-posix/index.html"

python3 -m http.server -d "$WORKSPACE/twister-out/coverage-posix"

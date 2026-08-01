#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Run the POSIX test suites under AddressSanitizer on native_sim and
# summarize the results, mirroring the twister-sanitizer CI job:
#
#   ./scripts/ci/asan.sh [extra runci.sh / twister args]
#
# ubsan.sh runs the same flow with SANITIZER=ubsan (UndefinedBehaviorSanitizer).
#
# Outputs (in the workspace topdir):
#   twister-out*/           twister results
#   asan-findings.json      sanitizer-implicated implementation functions
#   asan-summary.json       badge summary (single asan scenario class)
#
# The summary is not packed into doc/metrics/ automatically. To preview the
# badges in a local docs build (do not commit the result):
#   python3 scripts/ci/jsonball.py pack ../../../asan-summary.json \
#     -o doc/metrics/asan-summary.json.sh
#
# Prerequisites (once):
#   source ~/posix-next/zephyr/zephyr-env.sh

set -euo pipefail

SANITIZER="${SANITIZER:-asan}"

REALPATH="realpath"
SCRIPT_PATH="$($REALPATH "$(dirname "$0")")"
POSIX_NEXT_PATH="$($REALPATH "$SCRIPT_PATH"/../..)"
WORKSPACE_PATH="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"

# attribute UBSAN errors to functions; the twister handler appends its own
# log_path / halt_on_error settings to the inherited value
export UBSAN_OPTIONS="print_stacktrace=1:${UBSAN_OPTIONS:-}"

# a sanitizer error fails the corresponding twister instance; keep going so
# the findings and summary always reflect the run, then exit with twister's
# status
rc=0
CI_CONFIG_PROFILE="twister_${SANITIZER}" \
	"$SCRIPT_PATH/runci.sh" -i "$@" || rc=$?

python3 "$SCRIPT_PATH/sanitizer-findings.py" \
	--output "$WORKSPACE_PATH/${SANITIZER}-findings.json" \
	"$WORKSPACE_PATH"/twister-out*

# twister-out.N retry backups precede the final twister-out ('.' < '/'):
# the summarizer merges last-wins per (suite, platform)
inputs=()
for f in "$WORKSPACE_PATH"/twister-out.*/twister.json "$WORKSPACE_PATH"/twister-out/twister.json; do
	[ -f "$f" ] && inputs+=("$f")
done
python3 "$SCRIPT_PATH/twister-summarize.py" \
	--output "$WORKSPACE_PATH/${SANITIZER}-summary.json" \
	--commit "$(git -C "$POSIX_NEXT_PATH" rev-parse HEAD)" \
	--profile "twister_${SANITIZER}" \
	--force-variant "${SANITIZER}" \
	--findings "$WORKSPACE_PATH/${SANITIZER}-findings.json" \
	"${inputs[@]}"

echo "${SANITIZER}.sh: summary in $WORKSPACE_PATH/${SANITIZER}-summary.json"
exit "$rc"

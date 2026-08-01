#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Build the POSIX test suites on native_sim (--build-only, static_analysis
# profile) and run the clang static analyzer over the module implementation
# sources, mirroring the twister-static-analysis CI job:
#
#   ./scripts/ci/scan-build.sh [extra runci.sh / twister args]
#
# Output (in the workspace topdir):
#   static-analysis.json    findings snapshot (analyzed files + per-finding
#                           file, line, enclosing function, checker)
#
# The snapshot is not packed into doc/metrics/ automatically. To preview the
# badges in a local docs build (do not commit the result):
#   python3 scripts/ci/jsonball.py pack ../../../static-analysis.json \
#     -o doc/metrics/static-analysis.json.sh
#
# Prerequisites (once):
#   source ~/posix-next/zephyr/zephyr-env.sh
#   sudo apt install clang clang-tools   # clang + scan-build's analyze-build

set -euo pipefail

REALPATH="realpath"
SCRIPT_PATH="$($REALPATH "$(dirname "$0")")"
POSIX_NEXT_PATH="$($REALPATH "$SCRIPT_PATH"/../..)"
WORKSPACE_PATH="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"

CI_CONFIG_PROFILE=static_analysis \
	"$SCRIPT_PATH/runci.sh" -i "$@"

python3 "$SCRIPT_PATH/static-analysis.py" \
	--output "$WORKSPACE_PATH/static-analysis.json" \
	--commit "$(git -C "$POSIX_NEXT_PATH" rev-parse HEAD)" \
	"$WORKSPACE_PATH"/twister-out*

echo "scan-build.sh: findings in $WORKSPACE_PATH/static-analysis.json"

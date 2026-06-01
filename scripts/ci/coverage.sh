#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

REALPATH="realpath"

SCRIPT_PATH="$($REALPATH "$(dirname "$0")")"
POSIX_NEXT_PATH="$($REALPATH "$SCRIPT_PATH"/../..)"
WORKSPACE_PATH="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"

exec "$SCRIPT_PATH/runci.sh" \
  -i --force-color -N -v \
  --filter runnable \
  --coverage \
  --coverage-tool gcovr \
  --coverage-per-instance \
  --disable-coverage-aggregation \
  --coverage-basedir "$WORKSPACE_PATH" \
  -xCONFIG_TEST_EXTRA_STACK_SIZE=4096 \
  --timeout-multiplier 2 \
  -e nano \
  "$@"

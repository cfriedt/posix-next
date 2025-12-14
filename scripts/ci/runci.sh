#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

PR_DEST="origin/main"

SCRIPT_PATH="$(realpath -s "$(dirname "$0")")"

POSIX_NEXT_PATH="$(realpath -s "$SCRIPT_PATH"/../..)"
WORKSPACE_PATH="$(realpath -s "$POSIX_NEXT_PATH"/../../..)"
ZEPHYR_BASE="$WORKSPACE_PATH/zephyr"

PLATFORMS=( \
  mps2/an385 \
  native_sim \
  qemu_cortex_a53 \
  qemu_riscv64 \
  qemu_x86 \
  qemu_x86_64 \
)
ROOTS=( \
  $POSIX_NEXT_PATH/samples/posix \
  $POSIX_NEXT_PATH/tests/posix \
  $ZEPHYR_BASE/samples/net \
  $ZEPHYR_BASE/samples/subsys/shell/shell_module \
  $ZEPHYR_BASE/tests/net \
  $ZEPHYR_BASE/tests/lib/c_lib \
)

addprefix() {
  local result=""
  local prefix="$1"
  shift

  while [ $# -gt 0 ]; do
    result+="$prefix $1 "
    shift
  done

  echo $result
}

platforms() {
  addprefix "-p" "${PLATFORMS[@]}"
}

roots() {
  addprefix "-T" "${ROOTS[@]}"
}

cd $POSIX_NEXT_PATH
rm -f west_old.yml
cp west.yml west_old.yml

EVENT_NAME=""
if [ "$(git diff --name-only $PR_DEST..)" == "" ]; then
  EVENT_NAME="push"
else
  EVENT_NAME="pull_request"
fi

cd "$ZEPHYR_BASE"

if [ "$EVENT_NAME" = "pull_request" ]; then

./scripts/ci/test_plan.py -r "$POSIX_NEXT_PATH" \
  -o "$POSIX_NEXT_PATH"/testplan.json \
  -c $PR_DEST.. --pull-request \
  $(platforms) \
  $(roots)

./scripts/twister \
  -i -c \
  -O "$POSIX_NEXT_PATH"/twister-out \
  --load-tests "$POSIX_NEXT_PATH"/testplan.json \
  $(platforms) \
  $(roots)

else

./scripts/twister \
  -i -c \
  -O "$POSIX_NEXT_PATH"/twister-out \
  $(platforms) \
  $(roots)
fi

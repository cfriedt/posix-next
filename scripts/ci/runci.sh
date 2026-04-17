#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

PR_DEST="origin/main"

SCRIPT_PATH="$(realpath -s "$(dirname "$0")")"

POSIX_NEXT_PATH="$(realpath -s "$SCRIPT_PATH"/../..)"
WORKSPACE_PATH="$(realpath -s "$POSIX_NEXT_PATH"/../../..)"
ZEPHYR_BASE="$WORKSPACE_PATH/zephyr"

DEFAULT_PLATFORMS=( \
  mps2/an385 \
  native_sim \
  qemu_cortex_a53 \
  qemu_riscv64 \
  qemu_x86 \
  qemu_x86_64 \
)
DEFAULT_ROOTS=( \
  $POSIX_NEXT_PATH/samples/posix \
  $POSIX_NEXT_PATH/tests/benchmarks/posix \
  $POSIX_NEXT_PATH/tests/posix \
  $ZEPHYR_BASE/zephyr/kernel/threads/thread_apis \
  $ZEPHYR_BASE/zephyr/kernel/signal \
  $ZEPHYR_BASE/samples/net \
  $ZEPHYR_BASE/samples/subsys/shell/shell_module \
  $ZEPHYR_BASE/tests/net \
  $ZEPHYR_BASE/tests/lib/c_lib \
)
declare -a PLATFORMS
declare -a ROOTS
declare -a ARGS

addprefix() {
  local prefix="$1"
  local result=""
  shift

  while [ $# -gt 0 ]; do
    result+="$prefix "$'\n'
    result+="$1"$'\n'
    shift
  done

  printf '%s' "$result"
}

platforms() {
  if [ ${#PLATFORMS[@]} -gt 0 ]; then
    addprefix "-p" "${PLATFORMS[@]}"
  fi
}

roots() {
  if [ ${#ROOTS[@]} -gt 0 ]; then
    addprefix "-T" "${ROOTS[@]}"
  fi
}

usage() {
  echo "Usage: $0 [options]"
  echo "Options:"
  echo "  -h, --help    Show this help message and exit"
  echo "  -p, --platform <platform>  Specify a platform to test (can be used multiple times)"
  echo "  -T, --test-root <root>     Specify a test root to include (can be used multiple times)"
}

while [ $# -gt 0 ]; do
  case "$1" in
    -h|--help)
      usage
      exit 0
      ;;
    -p|--platform)
      if [ -z "$2" ]; then
        echo "Error: Missing argument for $1"
        usage
        exit 1
      fi
      PLATFORMS+=("$2")
      shift
      ;;
    -T|--test-root)
      if [ -z "$2" ]; then
        echo "Error: Missing argument for $1"
        usage
        exit 1
      fi
      # Convert to absolute path using realpath
      root_path="$(realpath -s "$2" 2>/dev/null || echo "$2")"
      ROOTS+=("$root_path")
      shift
      ;;
    *)
      ARGS+=("$1")
      ;;
  esac
  shift
done

if [ ${#PLATFORMS[@]} -eq 0 ]; then
  PLATFORMS=("${DEFAULT_PLATFORMS[@]}")
fi
mapfile -t PLATFORMS < <(addprefix "-p" "${PLATFORMS[@]}")

if [ ${#ROOTS[@]} -eq 0 ]; then
  ROOTS=("${DEFAULT_ROOTS[@]}")
fi

mapfile -t ROOTS < <(addprefix "-T" "${ROOTS[@]}")

cd $POSIX_NEXT_PATH
rm -f west_old.yml
cp west.yml west_old.yml

EVENT_NAME=""
if [ "$(git diff --name-only $PR_DEST..)" == "" ]; then
  EVENT_NAME="push"
else
  if [ -z "$PS1" ]; then
    EVENT_NAME="CLI"
  else
    EVENT_NAME="pull_request"
  fi
fi

cd "$ZEPHYR_BASE"

if [ "$EVENT_NAME" = "pull_request" ]; then

./scripts/ci/test_plan.py -r "$POSIX_NEXT_PATH" \
  -o "$POSIX_NEXT_PATH"/testplan.json \
  -c $PR_DEST.. --pull-request \
  "${PLATFORMS[@]}" \
  "${ROOTS[@]}"

./scripts/twister \
  -i -c \
  -O "$POSIX_NEXT_PATH"/twister-out \
  --load-tests "$POSIX_NEXT_PATH"/testplan.json \
  ${PLATFORMS[@]} \
  ${ROOTS[@]} \
  ${ARGS[@]}
else

./scripts/twister \
  -i -c \
  -O "$POSIX_NEXT_PATH"/twister-out \
  ${PLATFORMS[@]} \
  ${ROOTS[@]} \
  ${ARGS[@]}
fi

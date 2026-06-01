#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

if [ -n "${ZEPHYR_SDK_INSTALL_DIR:-}" ]; then
  for d in \
    "${ZEPHYR_SDK_INSTALL_DIR}/aarch64-zephyr-elf/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/riscv64-zephyr-elf/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/x86_64-zephyr-elf/bin"
  do
    if [ -d "$d" ]; then
      export PATH="$d:$PATH"
    fi
  done
fi

REALPATH="realpath"

PR_DEST="origin/main"

SCRIPT_PATH="$($REALPATH "$(dirname "$0")")"

POSIX_NEXT_PATH="$($REALPATH "$SCRIPT_PATH"/../..)"
WORKSPACE_PATH="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"
ZEPHYR_BASE="$WORKSPACE_PATH/zephyr"
CI_CONFIG="$POSIX_NEXT_PATH"/.github/ci-config.json
HAVE_PLAN=0

declare -a DEFAULT_PLATFORMS=()
declare -a DEFAULT_ROOTS=()
declare -a PLATFORMS=()
declare -a ROOTS=()
declare -a ARGS=()

load_defaults_from_config() {
  local profile="$1"
  local rel abs

  if [ ! -f "$CI_CONFIG" ]; then
    echo "Missing $CI_CONFIG" >&2
    exit 1
  fi
  if ! command -v jq >/dev/null 2>&1; then
    echo "jq is required to read $CI_CONFIG" >&2
    exit 1
  fi

  if [ "$(uname -s)" = Darwin ]; then
    mapfile -t DEFAULT_PLATFORMS < <(
      jq -r ".${profile}.platforms[] | select(startswith(\"native_sim\") | not)" \
        "$CI_CONFIG"
    )
  else
    mapfile -t DEFAULT_PLATFORMS < <(
      jq -r ".${profile}.platforms[]" "$CI_CONFIG"
    )
  fi

  while IFS= read -r rel; do
    if [[ "$rel" = /* ]]; then
      abs="$rel"
    elif [ -d "$WORKSPACE_PATH/$rel" ]; then
      abs="$WORKSPACE_PATH/$rel"
    else
      abs="$($REALPATH "$WORKSPACE_PATH/$rel" 2>/dev/null || echo "$WORKSPACE_PATH/$rel")"
    fi
    DEFAULT_ROOTS+=("$abs")
  done < <(jq -r ".${profile}.roots[]" "$CI_CONFIG")
}

usage() {
  "$ZEPHYR_BASE/scripts/twister" --help
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
      root_path="$($REALPATH "$2" 2>/dev/null || echo "$2")"
      ROOTS+=("$root_path")
      shift
      ;;
    -F|--load-tests)
      if [ -z "$2" ]; then
        echo "Error: Missing argument for $1" >&2
        usage
        exit 1
      fi
      HAVE_PLAN=1
      ARGS+=("$1" "$2")
      shift
      ;;
    *)
      ARGS+=("$1")
      ;;
  esac
  shift
done

cd "$POSIX_NEXT_PATH"
rm -f west_old.yml
cp west.yml west_old.yml

cd "$ZEPHYR_BASE"

if [ ${#PLATFORMS[@]} -eq 0 ] || [ ${#ROOTS[@]} -eq 0 ]; then
  profile=twister_nightly
  for arg in "${ARGS[@]}"; do
    if [ "$arg" = --coverage ]; then
      profile=coverage_nightly
      break
    fi
  done
  load_defaults_from_config "$profile"
fi

if [ ${#PLATFORMS[@]} -eq 0 ]; then
  PLATFORMS=("${DEFAULT_PLATFORMS[@]}")
fi

_tmp=("${PLATFORMS[@]}")
PLATFORMS=()
for p in "${_tmp[@]}"; do
  PLATFORMS+=(-p "$p")
done
unset _tmp

if [ ${#ROOTS[@]} -eq 0 ]; then
  ROOTS=("${DEFAULT_ROOTS[@]}")
fi

_tmp=("${ROOTS[@]}")
ROOTS=()
for r in "${_tmp[@]}"; do
  ROOTS+=(-T "$r")
done
unset _tmp

cd $POSIX_NEXT_PATH
rm -f west_old.yml
cp west.yml west_old.yml

EVENT_NAME=""
if [ "$(git -C "$POSIX_NEXT_PATH" diff --name-only "$PR_DEST".. 2>/dev/null)" = "" ]; then
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
fi

twister_cmd=(
  ./scripts/twister
  -c
)

if [ $HAVE_PLAN -eq 1 ]; then
  twister_cmd+=(
    "${PLATFORMS[@]}"
    "${ROOTS[@]}"
  )
fi

if [ "$EVENT_NAME" = "pull_request" ]; then
  twister_cmd+=(--load-tests "$POSIX_NEXT_PATH"/testplan.json)
fi

twister_cmd+=("${ARGS[@]}")

exec "${twister_cmd[@]}"

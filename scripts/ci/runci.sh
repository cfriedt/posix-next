#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -e

if [ -n "${ZEPHYR_SDK_INSTALL_DIR:-}" ]; then
  for d in \
    "${ZEPHYR_SDK_INSTALL_DIR}/aarch64-zephyr-elf/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/riscv64-zephyr-elf/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/x86_64-zephyr-elf/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/gnu/aarch64-zephyr-elf/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/gnu/arm-zephyr-eabi/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/gnu/riscv64-zephyr-elf/bin" \
    "${ZEPHYR_SDK_INSTALL_DIR}/gnu/x86_64-zephyr-elf/bin"
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
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"
CI_CONFIG_PROFILE="${CI_CONFIG_PROFILE:-twister_nightly}"
HAVE_PLAN=0

command -v jq >/dev/null 2>&1 || {
  echo "jq is required to read $CI_CONFIG" >&2
  exit 1
}

tags_rel=$(jq -r '.paths.tags // ".github/tags.yaml"' "$CI_CONFIG")
ignore_rel=$(jq -r '.paths.twister_ignore // ".github/twister_ignore.txt"' "$CI_CONFIG")
TAGS_CONFIG="$POSIX_NEXT_PATH/$tags_rel"
TWISTER_IGNORE="$POSIX_NEXT_PATH/$ignore_rel"

mapfile -t DEFAULT_PLATFORMS < <(
  jq -r --arg os "$(uname -s)" --arg profile "$CI_CONFIG_PROFILE" '
    .[$profile].platforms[]
    | if $os == "Darwin" then select(startswith("native_sim") | not) else . end
  ' "$CI_CONFIG"
)
mapfile -t DEFAULT_ROOTS < <(
  jq -r --arg profile "$CI_CONFIG_PROFILE" '.[$profile].roots[]' "$CI_CONFIG"
)
mapfile -t DEFAULT_ARGS < <(
  jq -r --arg profile "$CI_CONFIG_PROFILE" '
    (.[$profile].run_args // .[$profile].args // [])[]
  ' "$CI_CONFIG"
)

declare -a PLATFORMS=()
declare -a ROOTS=()
declare -a ARGS=()

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
TEST_PLAN_ROOTS=()
for r in "${_tmp[@]}"; do
  if [[ "$r" = /* ]]; then
    ROOTS+=(-T "$r")
    TEST_PLAN_ROOTS+=(-T "$r")
  else
    ROOTS+=(-T "$r")
    TEST_PLAN_ROOTS+=(-T "$WORKSPACE_PATH/$r")
  fi
done
unset _tmp

cd "$POSIX_NEXT_PATH"
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

if [ "$EVENT_NAME" = "pull_request" ]; then
  "$ZEPHYR_BASE"/scripts/ci/test_plan.py -r "$POSIX_NEXT_PATH" \
    -o "$POSIX_NEXT_PATH"/testplan.json \
    -c $PR_DEST.. --pull-request \
    --alt-tags "$TAGS_CONFIG" \
    --ignore-path "$TWISTER_IGNORE" \
    "${PLATFORMS[@]}" \
    "${TEST_PLAN_ROOTS[@]}"
fi

twister_cmd=(
  "$ZEPHYR_BASE"/scripts/twister
  -c
)

# Always pass -p: with --load-tests, Twister still needs options.platform so
# --coverage can populate coverage_platform and enable CONFIG_COVERAGE=y.
twister_cmd+=("${PLATFORMS[@]}")

twister_cmd+=("${ROOTS[@]}")

if [ "$EVENT_NAME" = "pull_request" ]; then
  twister_cmd+=(--load-tests "$POSIX_NEXT_PATH"/testplan.json)
fi

needs_subset=0
for arg in "${DEFAULT_ARGS[@]}" "${ARGS[@]}"; do
  if [ "$arg" = "--shuffle-tests" ]; then
    needs_subset=1
    break
  fi
done

has_subset=0
for arg in "${ARGS[@]}"; do
  if [ "$arg" = "--subset" ] || [ "$arg" = "-B" ]; then
    has_subset=1
    break
  fi
done

if [ "$needs_subset" -eq 1 ] && [ "$has_subset" -eq 0 ]; then
  ARGS=(--subset 1/1 "${ARGS[@]}")
fi

twister_cmd+=("${DEFAULT_ARGS[@]}")
twister_cmd+=("${ARGS[@]}")

cd "$WORKSPACE_PATH"
exec "${twister_cmd[@]}"

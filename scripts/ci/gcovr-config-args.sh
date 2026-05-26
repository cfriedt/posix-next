#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Load gcovr --filter args from ci-config.json.
#
# Usage:
#   source /path/to/gcovr-config-args.sh
#   gcovr_load_filter_args filter_args workspace "$CI_CONFIG"
#   gcovr_load_filter_args filter_args posix "$CI_CONFIG"

gcovr_load_filter_args() {
  local -n _out="$1"
  local scope="${2:?scope required (workspace|posix)}"
  local ci_config="${3:?CI_CONFIG required}"
  local key=""

  case "$scope" in
    workspace) key=gcovr_workspace_filters ;;
    posix) key=gcovr_posix_filters ;;
    *)
      echo "gcovr_load_filter_args: unknown scope: $scope" >&2
      return 1
      ;;
  esac

  _out=()
  while IFS= read -r path; do
    [ -n "$path" ] || continue
    _out+=(--filter "$path")
  done < <(jq -r ".coverage_report.${key}[]? // empty" "$ci_config")
}

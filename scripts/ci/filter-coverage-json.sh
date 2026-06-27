#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Filter a merged gcovr JSON snapshot by scope paths from ci-config.json.

set -euo pipefail

usage() {
  echo "Usage: $0 --scope workspace|posix --ci-config PATH INPUT.json OUTPUT.json" >&2
  exit 2
}

scope=""
ci_config=""
input=""
output=""

while [ $# -gt 0 ]; do
  case "$1" in
    --scope)
      scope="${2:?}"
      shift 2
      ;;
    --ci-config)
      ci_config="${2:?}"
      shift 2
      ;;
    -h|--help)
      usage
      ;;
    -*)
      echo "Unknown option: $1" >&2
      usage
      ;;
    *)
      if [ -z "$input" ]; then
        input="$1"
      elif [ -z "$output" ]; then
        output="$1"
      else
        echo "Unexpected argument: $1" >&2
        usage
      fi
      shift
      ;;
  esac
done

if [ -z "$scope" ] || [ -z "$ci_config" ] || [ -z "$input" ] || [ -z "$output" ]; then
  usage
fi

command -v jq >/dev/null 2>&1 || {
  echo "jq is required" >&2
  exit 1
}

case "$scope" in
  workspace) key=gcovr_workspace_filters ;;
  posix) key=gcovr_posix_filters ;;
  *)
    echo "filter-coverage-json: unknown scope: $scope" >&2
    exit 1
    ;;
esac

prefixes_json=$(jq -c ".coverage_report.${key} // []" "$ci_config")
if [ "$prefixes_json" = "[]" ]; then
  echo "filter-coverage-json: no filters for scope ${scope}" >&2
  exit 1
fi

mkdir -p "$(dirname "$output")"

jq --argjson prefixes "$prefixes_json" '
  .files |= map(
    select(.file as $f | ($prefixes | any(. as $p | $f == $p or ($f | startswith($p + "/")))))
  )
' "$input" > "$output"

if [ "$(jq '.files | length' "$output")" -eq 0 ]; then
  echo "filter-coverage-json: filter is empty ($output)" >&2
  exit 1
fi

file_count=$(jq '.files | length' "$output")
echo "filter-coverage-json: wrote ${file_count} files to $output (scope=${scope})"

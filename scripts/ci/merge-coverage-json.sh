#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Merge pre-filtered gcovr JSON traces into one output file.

set -euo pipefail

usage() {
  echo "Usage: $0 --workspace ROOT --output OUT.json --ci-config PATH -- TRACE_JSON..." >&2
  exit 2
}

workspace=""
output=""
ci_config=""
tracefiles=()

while [ $# -gt 0 ]; do
  case "$1" in
    --workspace)
      workspace="${2:?}"
      shift 2
      ;;
    --output)
      output="${2:?}"
      shift 2
      ;;
    --ci-config)
      ci_config="${2:?}"
      shift 2
      ;;
    --)
      shift
      tracefiles=("$@")
      break
      ;;
    -h|--help)
      usage
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      ;;
  esac
done

if [ -z "$workspace" ] || [ -z "$output" ] || [ -z "$ci_config" ] || \
   [ ${#tracefiles[@]} -eq 0 ]; then
  usage
fi

command -v gcovr jq >/dev/null 2>&1 || {
  echo "gcovr and jq are required" >&2
  exit 1
}

mkdir -p "$(dirname "$output")"
cd "$workspace"

gcovr_args=()
while IFS= read -r a; do gcovr_args+=("$a"); done \
  < <(jq -r '.coverage_report.gcovr_args[]? // empty' "$ci_config")

trace_args=()
for f in "${tracefiles[@]}"; do
  trace_args+=(--add-tracefile "$f")
done

gcovr -r "$workspace" \
  "${gcovr_args[@]}" \
  "${trace_args[@]}" \
  --json "$output"

if [ "$(jq '.files | length' "$output")" -eq 0 ]; then
  echo "merge-coverage-json: merge is empty ($output)" >&2
  exit 1
fi

echo "merge-coverage-json: wrote $(jq '.files | length' "$output") files to $output"

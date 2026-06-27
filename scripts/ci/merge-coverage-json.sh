#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Merge pre-filtered gcovr JSON traces into one output file.
# Uses hierarchical batching so large local Twister runs do not OOM gcovr.

set -euo pipefail

usage() {
  echo "Usage: $0 --workspace ROOT --output OUT.json --ci-config PATH \\" >&2
  echo "  [--filter-scope SCOPE] -- TRACE_JSON..." >&2
  echo "  SCOPE: workspace | posix | none (default: none)" >&2
  exit 2
}

workspace=""
output=""
ci_config=""
filter_scope="none"
tracefiles=()
merge_batch_size="${MERGE_BATCH_SIZE:-64}"

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
    --filter-scope)
      filter_scope="${2:?}"
      shift 2
      ;;
    --merge-batch-size)
      merge_batch_size="${2:?}"
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

# shellcheck source=gcovr-config-args.sh
source "$(dirname "$0")/gcovr-config-args.sh"

gcovr_args=()
while IFS= read -r a; do gcovr_args+=("$a"); done \
  < <(jq -r '.coverage_report.gcovr_args[]? // empty' "$ci_config")

filter_args=()
case "$filter_scope" in
  workspace)
    gcovr_load_filter_args filter_args workspace "$ci_config"
    ;;
  posix)
    gcovr_load_filter_args filter_args posix "$ci_config"
    ;;
  none)
    ;;
  *)
    echo "merge-coverage-json: unknown filter scope: $filter_scope" >&2
    exit 1
    ;;
esac

merge_gcovr_traces() {
  local out="$1"
  shift
  local inputs=("$@")
  local trace_args=()
  local f

  for f in "${inputs[@]}"; do
    trace_args+=(--add-tracefile "$f")
  done

  gcovr -r "$workspace" \
    "${gcovr_args[@]}" \
    "${filter_args[@]}" \
    "${trace_args[@]}" \
    --json "$out"
}

merge_many() {
  local out="$1"
  shift
  local inputs=("$@")
  local count=${#inputs[@]}

  if [ "$count" -eq 0 ]; then
    echo "merge-coverage-json: no trace inputs for $out" >&2
    return 1
  fi

  if [ "$count" -le "$merge_batch_size" ]; then
    merge_gcovr_traces "$out" "${inputs[@]}"
    return
  fi

  local tmpdir
  tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/merge-coverage.XXXXXX")
  local intermediates=()
  local i=0 batch=0 slice=()

  echo "merge-coverage-json: batching ${count} trace(s) (batch size ${merge_batch_size})"
  while [ "$i" -lt "$count" ]; do
    slice=("${inputs[@]:i:merge_batch_size}")
    local intermediate="${tmpdir}/batch-${batch}.json"
    merge_many "$intermediate" "${slice[@]}"
    intermediates+=("$intermediate")
    i=$((i + merge_batch_size))
    batch=$((batch + 1))
  done

  merge_many "$out" "${intermediates[@]}"
  rm -rf "$tmpdir"
}

merge_many "$output" "${tracefiles[@]}"

if [ "$(jq '.files | length' "$output")" -eq 0 ]; then
  echo "merge-coverage-json: merge is empty ($output)" >&2
  exit 1
fi

file_count=$(jq '.files | length' "$output")
trace_count=${#tracefiles[@]}
echo "merge-coverage-json: wrote ${file_count} files to $output (from ${trace_count} trace(s))"

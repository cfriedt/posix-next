#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Merge shard gcovr JSON traces into full + posix snapshots and provenance.

set -euo pipefail

usage() {
  echo "Usage: $0 --workspace ROOT --output-dir DIR --run-url URL --commit SHA \\" >&2
  echo "         --ci-config PATH -- TRACE_JSON..." >&2
  exit 2
}

workspace=""
output_dir=""
run_url=""
commit=""
ci_config=""
tracefiles=()

while [ $# -gt 0 ]; do
  case "$1" in
    --workspace)
      workspace="${2:?}"
      shift 2
      ;;
    --output-dir)
      output_dir="${2:?}"
      shift 2
      ;;
    --run-url)
      run_url="${2:?}"
      shift 2
      ;;
    --commit)
      commit="${2:?}"
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

if [ -z "$workspace" ] || [ -z "$output_dir" ] || [ -z "$run_url" ] || \
   [ -z "$commit" ] || [ -z "$ci_config" ] || [ ${#tracefiles[@]} -eq 0 ]; then
  usage
fi

command -v gcovr jq >/dev/null 2>&1 || {
  echo "gcovr and jq are required" >&2
  exit 1
}

mkdir -p "$output_dir"
cd "$workspace"

gcovr_args=()
while IFS= read -r a; do gcovr_args+=("$a"); done \
  < <(jq -r '.coverage_report.gcovr_args[]? // empty' "$ci_config")

trace_args=()
for f in "${tracefiles[@]}"; do
  trace_args+=(--add-tracefile "$f")
done

full_json="${output_dir}/coverage-full.json"
posix_json="${output_dir}/coverage-posix.json"
provenance_json="${output_dir}/coverage-provenance.json"

gcovr -r "$workspace" \
  "${gcovr_args[@]}" \
  "${trace_args[@]}" \
  --json "$full_json"

if [ "$(jq '.files | length' "$full_json")" -eq 0 ]; then
  echo "merge-coverage-json: full merge is empty" >&2
  exit 1
fi

filter_args=()
while IFS= read -r p; do filter_args+=(--filter "$p"); done \
  < <(jq -r '.coverage_report.gcovr_filters[]? // empty' "$ci_config")

gcovr -r "$workspace" \
  "${gcovr_args[@]}" \
  "${trace_args[@]}" \
  "${filter_args[@]}" \
  --json "$posix_json"

if [ "$(jq '.files | length' "$posix_json")" -eq 0 ]; then
  echo "merge-coverage-json: posix merge is empty" >&2
  exit 1
fi

line_stats() {
  local file="$1"
  jq -r '
    def hits:
      [(.files[]?.lines[]?.count // empty) | select(. > 0)] | length;
    def total:
      [(.files[]?.lines[]?) | select(. != null)] | length;
    [hits, total] | @tsv
  ' "$file"
}

read -r full_hits full_total < <(line_stats "$full_json")
read -r posix_hits posix_total < <(line_stats "$posix_json")

full_pct=$(awk -v h="$full_hits" -v t="$full_total" 'BEGIN { if (t > 0) printf "%.1f", h * 100 / t; else print "0.0" }')
posix_pct=$(awk -v h="$posix_hits" -v t="$posix_total" 'BEGIN { if (t > 0) printf "%.1f", h * 100 / t; else print "0.0" }')

jq -n \
  --arg url "$run_url" \
  --arg sha "$commit" \
  --arg created "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
  --arg full_pct "$full_pct" \
  --arg posix_pct "$posix_pct" \
  --argjson full_hits "$full_hits" \
  --argjson full_total "$full_total" \
  --argjson posix_hits "$posix_hits" \
  --argjson posix_total "$posix_total" \
  --argjson full_files "$(jq '.files | length' "$full_json")" \
  --argjson posix_files "$(jq '.files | length' "$posix_json")" \
  '{
    workflow_run_url: $url,
    commit: $sha,
    created_at: $created,
    full: {
      line_coverage: $full_pct,
      line_hits: $full_hits,
      line_total: $full_total,
      file_count: $full_files
    },
    posix: {
      line_coverage: $posix_pct,
      line_hits: $posix_hits,
      line_total: $posix_total,
      file_count: $posix_files
    }
  }' > "$provenance_json"

echo "merge-coverage-json: full ${full_pct}% (${full_hits}/${full_total}), posix ${posix_pct}% (${posix_hits}/${posix_total})"

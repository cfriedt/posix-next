#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -euo pipefail

usage() {
  echo "Usage: $0 [--shard N] [--ratio R] [--output FILE] TWISTER_OUT_DIR" >&2
  exit 2
}

SCRIPT_PATH="$(realpath "$(dirname "$0")")"
POSIX_NEXT_PATH="$(realpath "$SCRIPT_PATH/../..")"
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"

shard=""
ratio=""
output=""

while [ $# -gt 0 ]; do
  case "$1" in
    --shard)
      shard="${2:?}"
      shift 2
      ;;
    --ratio)
      ratio="${2:?}"
      shift 2
      ;;
    --output)
      output="${2:?}"
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
      break
      ;;
  esac
done

twister_out="${1:?TWISTER_OUT_DIR required}"
twister_json="$twister_out/twister.json"

if [ -z "$ratio" ]; then
  ratio=$(jq -r '.coverage_nightly.config_failure_ratio // 0.5' "$CI_CONFIG")
fi

command -v jq >/dev/null 2>&1 || {
  echo "jq is required" >&2
  exit 1
}

configs_total=0
configs_failed=0
line_hits=0
non_empty_traces=0
shard_ok=true
reason="ok"

if [ ! -f "$twister_json" ]; then
  shard_ok=false
  reason="missing twister.json"
else
  read -r configs_total configs_failed < <(
    jq -r '
      [.testsuites[]? | select(.status == "passed" or .status == "failed" or .status == "error")] as $executed
      | ($executed | length) as $total
      | ($executed | map(select(.status == "failed" or .status == "error")) | length) as $failed
      | "\($total) \($failed)"
    ' "$twister_json"
  )

  if [ "$configs_total" -eq 0 ]; then
    shard_ok=false
    reason="no executed configurations"
  elif awk -v failed="$configs_failed" -v total="$configs_total" -v ratio="$ratio" \
      'BEGIN { exit !(total > 0 && failed / total >= ratio) }'; then
    shard_ok=false
    reason="config failure ratio >= ${ratio}"
  else
    reason="config failure ratio below ${ratio}"
  fi
fi

merged_cov="$twister_out/coverage-full.json"
if [ -f "$merged_cov" ]; then
  line_hits=$(jq '[.files[]?.lines[]?.count // empty | select(. > 0)] | length' "$merged_cov" 2>/dev/null || echo 0)
fi

shopt -s nullglob
while IFS= read -r trace; do
  file_count=$(jq -r '.files | length' "$trace" 2>/dev/null || echo 0)
  hit_count=$(jq -r '
    [(.files[]?.lines[]?.count // empty),
     (.files[]?.functions[]?.execution_count // empty)]
    | map(select(. != null and . > 0)) | length
  ' "$trace" 2>/dev/null || echo 0)
  if [ "$file_count" -gt 0 ] && [ "$hit_count" -gt 0 ]; then
    non_empty_traces=$((non_empty_traces + 1))
  fi
done < <(find "$twister_out" -name coverage.json -type f \
  ! -path "$twister_out/coverage-full.json" \
  ! -path "$twister_out/coverage-posix.json")

if [ "$shard_ok" = true ] && [ "$configs_total" -gt 0 ] && \
   [ "$line_hits" -eq 0 ] && [ "$non_empty_traces" -eq 0 ]; then
  shard_ok=false
  reason="no coverage hits recorded"
fi

if [ -n "$configs_total" ] && [ "$configs_total" -gt 0 ]; then
  observed_ratio=$(awk -v failed="$configs_failed" -v total="$configs_total" \
    'BEGIN { printf "%.6f", failed / total }')
else
  observed_ratio="0"
fi

status_json=$(jq -n \
  --argjson shard "${shard:-null}" \
  --argjson configs_total "$configs_total" \
  --argjson configs_failed "$configs_failed" \
  --argjson config_failure_threshold "$ratio" \
  --arg observed_config_failure_ratio "$observed_ratio" \
  --argjson line_hits "$line_hits" \
  --argjson non_empty_traces "$non_empty_traces" \
  --argjson shard_ok "$shard_ok" \
  --arg reason "$reason" \
  '{
    shard: $shard,
    configs_total: $configs_total,
    configs_failed: $configs_failed,
    config_failure_threshold: $config_failure_threshold,
    observed_config_failure_ratio: ($observed_config_failure_ratio | tonumber),
    line_hits: $line_hits,
    non_empty_traces: $non_empty_traces,
    shard_ok: $shard_ok,
    reason: $reason
  }')

if [ -n "$output" ]; then
  printf '%s\n' "$status_json" > "$output"
else
  printf '%s\n' "$status_json"
fi

echo "Shard ${shard:-?}: ${configs_failed}/${configs_total} configs failed (threshold ${ratio}, observed ${observed_ratio}), coverage line_hits=${line_hits} non_empty_traces=${non_empty_traces} -> shard_ok=${shard_ok} (${reason})"

if [ "$shard_ok" = true ]; then
  exit 0
fi
exit 1

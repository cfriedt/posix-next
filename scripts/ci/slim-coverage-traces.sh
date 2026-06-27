#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Trim per-test gcovr JSON traces to workspace filter paths (jq, no gcov re-run).

set -euo pipefail

usage() {
  echo "Usage: $0 TWISTER_OUT CI_CONFIG" >&2
  exit 2
}

twister_out="${1:?TWISTER_OUT required}"
ci_config="${2:?CI_CONFIG required}"

command -v jq >/dev/null 2>&1 || {
  echo "jq is required" >&2
  exit 1
}

prefixes_json=$(jq -c '.coverage_report.gcovr_workspace_filters // []' "$ci_config")
if [ "$prefixes_json" = "[]" ]; then
  echo "slim-coverage-traces: no gcovr_workspace_filters in $ci_config" >&2
  exit 1
fi

slimmed=0
skipped=0
reused=0

mapfile -t traces < <(
  find "$twister_out" -name coverage.json -type f \
    ! -path "$twister_out/coverage-full.json" \
    ! -path "$twister_out/coverage-posix.json"
)

for trace in "${traces[@]}"; do
  if [ ! -s "$trace" ]; then
    skipped=$((skipped + 1))
    continue
  fi

  out="${trace%.json}.workspace.json"
  if [ -f "$out" ] && [ "$out" -nt "$trace" ]; then
    reused=$((reused + 1))
    continue
  fi

  hit_count=$(jq -r '
    [(.files[]?.lines[]?.count // empty) | select(. > 0)] | length
  ' "$trace" 2>/dev/null || echo 0)
  if [ "$hit_count" -eq 0 ]; then
    skipped=$((skipped + 1))
    continue
  fi

  outside=$(jq -r --argjson prefixes "$prefixes_json" '
    [.files[]?.file // empty
     | select(
         ($prefixes | any(. as $p | . == $p or startswith($p + "/"))) | not
       )
    ] | length
  ' "$trace")

  if [ "$outside" -eq 0 ]; then
    if [ ! -f "$out" ] || [ "$trace" -nt "$out" ]; then
      cp -a "$trace" "$out"
      slimmed=$((slimmed + 1))
    else
      reused=$((reused + 1))
    fi
    continue
  fi

  if ! jq --argjson prefixes "$prefixes_json" '
    .files |= map(
      select(.file as $f | ($prefixes | any(. as $p | $f == $p or ($f | startswith($p + "/")))))
    )
  ' "$trace" > "${out}.tmp"; then
    echo "slim-coverage-traces: jq failed for ${trace}" >&2
    rm -f "${out}.tmp"
    skipped=$((skipped + 1))
    continue
  fi

  mv "${out}.tmp" "$out"
  slimmed=$((slimmed + 1))
done

echo "slim-coverage-traces: slimmed ${slimmed}, reused ${reused}, skipped ${skipped}"

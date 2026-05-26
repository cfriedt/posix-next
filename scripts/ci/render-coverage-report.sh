#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Merge shard gcovr trace JSON, render HTML, badge, and provenance into OUT_DIR.

set -euo pipefail

usage() {
  echo "Usage: $0 --workspace ROOT --output-dir DIR --scope SCOPE \\" >&2
  echo "         --run-url URL --commit SHA --ci-config PATH --manifest-path PATH \\" >&2
  echo "         [--filter PATH]... -- TRACE_JSON..." >&2
  exit 2
}

workspace=""
output_dir=""
scope=""
run_url=""
commit=""
ci_config=""
manifest_path=""
filter_args=()
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
    --scope)
      scope="${2:?}"
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
    --manifest-path)
      manifest_path="${2:?}"
      shift 2
      ;;
    --filter)
      filter_args+=(--filter "${2:?}")
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

if [ -z "$workspace" ] || [ -z "$output_dir" ] || [ -z "$scope" ] || \
   [ -z "$run_url" ] || [ -z "$commit" ] || [ -z "$ci_config" ] || \
   [ -z "$manifest_path" ] || [ ${#tracefiles[@]} -eq 0 ]; then
  usage
fi

command -v gcovr jq >/dev/null 2>&1 || {
  echo "gcovr and jq are required" >&2
  exit 1
}

cd "$workspace"

mkdir -p "$output_dir"
merged_json="${output_dir}/merged.json"

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
  "${filter_args[@]}" \
  --json "$merged_json"

if [ "$(jq '.files | length' "$merged_json")" -eq 0 ]; then
  echo "render-coverage-report (${scope}): merged coverage is empty" >&2
  exit 1
fi

tmp_merged="$(mktemp)"
jq '
  .files |= map(
    . as $f
    | ($f.lines // []) as $lines
    | ($f.functions // []) as $funcs
    | ($lines | map(select((.count // 0) > 0)) | length) as $line_hit
    | ($lines | length) as $line_total
    | ($funcs | map(select((.execution_count // 0) > 0)) | length) as $func_hit
    | ($funcs | length) as $func_total
    | $f + {
        line_executed: $line_hit,
        line_total: $line_total,
        line_percent: (if $line_total > 0 then ($line_hit * 100 / $line_total) else 0 end),
        function_executed: $func_hit,
        function_total: $func_total,
        function_percent: (if $func_total > 0 then ($func_hit * 100 / $func_total) else 0 end)
      }
  )
' "$merged_json" > "$tmp_merged"
mv "$tmp_merged" "$merged_json"

line_hit_sum=$(jq '[.files[].line_executed // 0] | add // 0' "$merged_json")
line_total_sum=$(jq '[.files[].line_total // 0] | add // 0' "$merged_json")
if [ "$line_total_sum" -eq 0 ]; then
  echo "render-coverage-report (${scope}): no instrumented lines" >&2
  exit 1
fi

html_json="$(mktemp)"
python3 - "$workspace" "$merged_json" "$html_json" <<'PY'
import json
import sys
from pathlib import Path

root = Path(sys.argv[1])
merged_path = Path(sys.argv[2])
html_path = Path(sys.argv[3])
data = json.loads(merged_path.read_text())

present = []
missing = 0
for entry in data.get("files", []):
    rel = entry.get("file", "")
    if rel and (root / rel).is_file():
        present.append(entry)
    else:
        missing += 1

if not present:
    print(
        f"render-coverage-report: no source files from merged.json exist under {root}",
        file=sys.stderr,
    )
    sys.exit(1)

data["files"] = present
html_path.write_text(json.dumps(data))
print(
    f"render-coverage-report: HTML trace uses {len(present)} file(s)"
    f" ({missing} missing on disk skipped)",
    file=sys.stderr,
)
PY

gcovr -r "$workspace" \
  "${gcovr_args[@]}" \
  --add-tracefile "$html_json" \
  --html-details "${output_dir}/index.html"

rm -f "$html_json"

"${workspace}/${manifest_path}/scripts/ci/patch-coverage-index.sh" \
  "$merged_json" "${output_dir}/index.html"

line_pct=$(awk -v hit="$line_hit_sum" -v total="$line_total_sum" \
  'BEGIN { printf "%.1f", hit * 100 / total }')
file_count=$(jq '.files | length' "$merged_json")
echo "${scope} line coverage: ${line_pct}% (${line_hit_sum}/${line_total_sum} instrumented lines in ${file_count} files)"

line_msg="${line_pct}%"
green=$(jq -r '.badge_thresholds.green' "$ci_config")
yellow=$(jq -r '.badge_thresholds.yellow' "$ci_config")
if awk "BEGIN {exit !($line_pct >= $green)}"; then
  color=brightgreen
elif awk "BEGIN {exit !($line_pct >= $yellow)}"; then
  color=yellow
else
  color=red
fi

jq -n --arg msg "$line_msg" --arg color "$color" \
  '{schemaVersion: 1, label: "coverage", message: $msg, color: $color}' \
  > "${output_dir}/badge.json"
jq -n \
  --arg url "$run_url" \
  --arg sha "$commit" \
  --arg scope "$scope" \
  --arg created "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
  --arg line "$line_msg" \
  '{workflow_run_url: $url, commit: $sha, scope: $scope, created_at: $created, line_coverage: $line}' \
  > "${output_dir}/provenance.json"

#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Download pr-testplan artifact from the Twister workflow for a PR head commit.

set -euo pipefail

HEAD_SHA="${1:?PR head commit SHA required}"
DEST="${2:-.}"

if ! command -v gh >/dev/null 2>&1; then
  echo "gh CLI is required" >&2
  exit 1
fi

artifact=pr-testplan
repo="${GITHUB_REPOSITORY:?GITHUB_REPOSITORY must be set}"

while IFS= read -r run_id; do
  [ -z "$run_id" ] && continue
  if gh api -H "Accept: application/vnd.github+json" \
    "/repos/${repo}/actions/runs/${run_id}/artifacts" \
    --paginate -q ".artifacts[] | select(.name == \"${artifact}\") | .id" \
    | head -1 | grep -q .; then
    rm -rf "${DEST}/testplan.json"
    gh run download "$run_id" -n "$artifact" -D "$DEST"
    if [ -f "${DEST}/testplan.json" ] \
      && jq -e '.testsuites' "${DEST}/testplan.json" >/dev/null 2>&1; then
      count=$(jq '.testsuites | length' "${DEST}/testplan.json")
      echo "Downloaded ${artifact} from Twister run ${run_id} (${count} test(s))"
      exit 0
    fi
  fi
done < <(gh run list \
  --repo "$repo" \
  --workflow twister.yml \
  --commit "$HEAD_SHA" \
  --limit 10 \
  --json databaseId \
  -q '.[].databaseId')

exit 1

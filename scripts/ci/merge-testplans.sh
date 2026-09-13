#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Merge twister test plans (--save-tests / test_plan.py output) into one:
# the first plan's metadata with every plan's testsuites concatenated.
#
# usage: merge-testplans.sh <out.json> <in.json>...

set -euo pipefail

out="$1"
shift

inputs=()
for f in "$@"; do
  if [ -s "$f" ]; then
    inputs+=("$f")
  fi
done

if [ ${#inputs[@]} -eq 0 ]; then
  echo '{"testsuites":[]}' > "$out"
  exit 0
fi

jq -s '.[0] * {testsuites: (map(.testsuites // []) | add)}' "${inputs[@]}" > "$out"

#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Print the platform groups a profile's test plan is generated from, one group
# per line, platforms space-separated. Twister builds a scenario keyed on
# platform_key once per key among the platforms of one planning run; every
# platform named in the profile's "plan_separately" list forms a group of its
# own, so it is planned against nobody else.
#
# usage: plan-groups.sh <ci-config.json> <profile> [platform...]
#   With no platforms given, the profile's "platforms" list is used.

set -euo pipefail

CI_CONFIG="$1"
PROFILE="$2"
shift 2

if [ $# -gt 0 ]; then
  platforms=("$@")
else
  mapfile -t platforms < <(jq -r --arg profile "$PROFILE" '.[$profile].platforms[]' "$CI_CONFIG")
fi
mapfile -t separate < <(jq -r --arg profile "$PROFILE" '.[$profile].plan_separately[]? // empty' "$CI_CONFIG")

rest=()
for p in "${platforms[@]}"; do
  alone=0
  for s in "${separate[@]}"; do
    if [ "$p" = "$s" ]; then
      alone=1
      break
    fi
  done
  if [ "$alone" -eq 1 ]; then
    echo "$p"
  else
    rest+=("$p")
  fi
done
if [ ${#rest[@]} -gt 0 ]; then
  echo "${rest[*]}"
fi

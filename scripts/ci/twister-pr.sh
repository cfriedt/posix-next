#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -euo pipefail

CHANGED_FILES="${1:-changed_files.json}"

SCRIPT_PATH="$(realpath "$(dirname "$0")")"

"$SCRIPT_PATH/gen-pr-testplan.sh" "$CHANGED_FILES"
exec "$SCRIPT_PATH/twister-pr-run.sh"

#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Run the POSIX test suites under UndefinedBehaviorSanitizer on native_sim
# and summarize the results, mirroring the twister-sanitizer CI job:
#
#   ./scripts/ci/ubsan.sh [extra runci.sh / twister args]
#
# Same flow as asan.sh (see there for outputs and prerequisites), with the
# twister_ubsan profile and a ubsan scenario class.

SANITIZER=ubsan exec "$(dirname "$0")/asan.sh" "$@"

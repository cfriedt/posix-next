#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors

# Print the tag of the browser QEMU image: a hash of everything that goes
# into building it. web-demo.yml reuses the published image when the tag
# exists and builds it otherwise; see build-qemu-wasm.sh.

set -euo pipefail

cd "$(dirname "$(realpath "$0")")"

cat Dockerfile build-qemu-wasm.sh | sha256sum | cut -c1-16

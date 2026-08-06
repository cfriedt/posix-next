#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Print the CI container image tag: a hash of every input baked into the
# image. west.yml pins the zephyr revision, so zephyr's requirements.txt and
# SDK_VERSION are covered transitively. ci-image.yml builds/pushes on a tag
# it has not published yet; consumer workflows resolve the same tag and fall
# back to plain runners when the image does not exist (e.g. a PR that edits
# west.yml before the post-merge image build runs).

set -euo pipefail

cd "$(dirname "$(realpath "$0")")/../.."

cat docker/ci/Dockerfile west.yml scripts/ci/requirements-coverage.txt \
  | sha256sum | cut -c1-16

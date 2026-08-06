# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# CI build environment for posix-next. Bakes everything setup-workspace
# otherwise reassembles per job: apt packages, a Python venv with the Zephyr
# requirements, the Zephyr SDK toolchains + host tools, and a pristine west
# workspace tarball at /repo-cache/west-workspace.tar.zst.
#
# Built by .github/workflows/ci-image.yml and tagged with the hash printed by
# scripts/ci/ci-image.sh; consumer jobs resolve that tag and enter the image
# via `container:`. setup-workspace detects POSIX_NEXT_CI_IMAGE=1 and takes
# its fast path instead of action-zephyr-setup.

FROM ubuntu:24.04

ARG TOOLCHAINS="aarch64-zephyr-elf arm-zephyr-eabi riscv64-zephyr-elf x86_64-zephyr-elf"
ARG YQ_VERSION=v4.44.3

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        ccache \
        cmake \
        curl \
        device-tree-compiler \
        file \
        g++-multilib \
        gh \
        git \
        gperf \
        jq \
        libc6-dev-i386 \
        ninja-build \
        openssl \
        python3 \
        python3-dev \
        python3-venv \
        sudo \
        wget \
        xz-utils \
        zstd \
    && rm -rf /var/lib/apt/lists/*

# mikefarah yq, matching the GitHub-hosted runner tool used by setup-workspace
RUN wget -q -O /usr/local/bin/yq \
        "https://github.com/mikefarah/yq/releases/download/${YQ_VERSION}/yq_linux_amd64" \
    && chmod +x /usr/local/bin/yq

ENV VIRTUAL_ENV=/opt/venv \
    PATH=/opt/venv/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin

RUN python3 -m venv /opt/venv && pip install --no-cache-dir -U pip wheel west

# Single layer so the throwaway /ws workspace does not bloat the image:
# west-update a pristine workspace, install its Python requirements, install
# the SDK it pins, then keep only the workspace tarball + bake metadata.
COPY west.yml scripts/ci/requirements-coverage.txt /tmp/bake/
RUN set -eux; \
    mkdir -p /ws/modules/lib/posix /ws/.west /repo-cache /opt/posix-next-ci; \
    cp /tmp/bake/west.yml /ws/modules/lib/posix/west.yml; \
    printf '[manifest]\npath = modules/lib/posix\nfile = west.yml\n' > /ws/.west/config; \
    cd /ws; \
    west update -o=--depth=1 -n; \
    pip install --no-cache-dir \
        -r zephyr/scripts/requirements.txt \
        -r /tmp/bake/requirements-coverage.txt; \
    SDK_VERSION="$(cat zephyr/SDK_VERSION)"; \
    SDK_FILE="zephyr-sdk-${SDK_VERSION}_linux-x86_64_minimal.tar.xz"; \
    wget -q "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/${SDK_FILE}"; \
    mkdir -p /opt/zephyr-sdk; \
    tar xf "${SDK_FILE}" -C /opt/zephyr-sdk --strip-components=1; \
    rm -f "${SDK_FILE}"; \
    cd /opt/zephyr-sdk; \
    for toolchain in ${TOOLCHAINS}; do ./setup.sh -t "${toolchain}"; done; \
    if [ ! -d sysroots ]; then ./setup.sh -h; fi; \
    ./setup.sh -c; \
    cd /ws; \
    tar -cf - .west $(west list | grep -v '^manifest' | awk '{print $2}') \
        | zstd -T0 -q -o /repo-cache/west-workspace.tar.zst; \
    sha256sum modules/lib/posix/west.yml | awk '{print $1}' > /opt/posix-next-ci/west.yml.sha256; \
    cp zephyr/SDK_VERSION /opt/posix-next-ci/SDK_VERSION; \
    rm -rf /ws /tmp/bake

ENV POSIX_NEXT_CI_IMAGE=1

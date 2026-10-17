#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors

# Build the QEMU system emulator as WebAssembly for the in-browser demo.
#
# QEMU does not build with emscripten on its own; the qemu-wasm fork
# (https://github.com/ktock/qemu-wasm) adds a WebAssembly TCG backend, a
# fiber coroutine backend and the browser glue. This script checks out the
# pinned revision, builds its emscripten environment image, then builds the
# emulator with scripts/web-demo/Dockerfile and extracts the outputs.
#
# usage: build-qemu-wasm.sh [-t riscv64|aarch64|x86_64] [-w workdir] [-i image] -o outdir
#
#   -o  directory that receives qemu-system-<target>.{js,wasm,worker.js} and
#       the terminal runtime (xterm.js, xterm-pty)
#   -w  where the qemu-wasm checkout lives (default: $TMPDIR/qemu-wasm)
#   -i  tag for the output image (default: posix-next/qemu-wasm:<tag>)
#   -t  guest architecture (default: riscv64, the toybox sample's board)
#
# Needs docker with BuildKit (the qemu-wasm Dockerfile uses heredocs).

set -euo pipefail

QEMU_WASM_REPO=https://github.com/ktock/qemu-wasm.git
QEMU_WASM_COMMIT=0ef7b4e2814b231705d8371dd7997f5b72e70baf

target=riscv64
workdir="${TMPDIR:-/tmp}/qemu-wasm"
outdir=
image=

while getopts "t:w:o:i:h" opt; do
  case "$opt" in
    t) target="$OPTARG" ;;
    w) workdir="$OPTARG" ;;
    o) outdir="$OPTARG" ;;
    i) image="$OPTARG" ;;
    *) sed -n '/^# usage/,/^$/p' "$0"; exit 2 ;;
  esac
done
[ -n "$outdir" ] || { echo "$0: -o outdir is required" >&2; exit 2; }

here="$(dirname "$(realpath "$0")")"
[ -n "$image" ] || image="posix-next/qemu-wasm:$("$here/qemu-wasm-image.sh")"
export DOCKER_BUILDKIT=1

if [ ! -d "$workdir/.git" ]; then
  git init -q "$workdir"
  git -C "$workdir" remote add origin "$QEMU_WASM_REPO"
fi
if ! git -C "$workdir" cat-file -e "${QEMU_WASM_COMMIT}^{commit}" 2>/dev/null; then
  git -C "$workdir" fetch -q --depth 1 origin "$QEMU_WASM_COMMIT"
fi
git -C "$workdir" checkout -q --detach "$QEMU_WASM_COMMIT"
# zlib.net only keeps the current release; the pinned version lives on GitHub
zlib_url='https://github.com/madler/zlib/releases/download/v$ZLIB_VERSION/zlib-$ZLIB_VERSION.tar.xz'
sed -i "s#https://zlib.net/zlib-\$ZLIB_VERSION.tar.xz#$zlib_url#" "$workdir/Dockerfile"

echo "building the qemu-wasm emscripten environment"
docker build -t qemu-wasm-env -f "$workdir/Dockerfile" "$workdir"

echo "building qemu-system-$target for the browser as $image"
docker build -t "$image" -f "$here/Dockerfile" --target out \
  --build-arg BASE_IMAGE=qemu-wasm-env --build-arg TARGET="$target" "$workdir"

"$here/extract-qemu-wasm.sh" -i "$image" -o "$outdir"

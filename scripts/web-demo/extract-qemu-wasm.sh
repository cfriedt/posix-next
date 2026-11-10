#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors

# Copy the browser QEMU build out of its image (see build-qemu-wasm.sh).
#
# usage: extract-qemu-wasm.sh -i image -o outdir

set -euo pipefail

image=
outdir=
while getopts "i:o:h" opt; do
  case "$opt" in
    i) image="$OPTARG" ;;
    o) outdir="$OPTARG" ;;
    *) sed -n '/^# usage/,/^$/p' "$0"; exit 2 ;;
  esac
done
if [ -z "$image" ] || [ -z "$outdir" ]; then
  echo "$0: -i image and -o outdir are required" >&2
  exit 2
fi

mkdir -p "$outdir"
id=$(docker create "$image")
trap 'docker rm -f "$id" >/dev/null' EXIT
docker cp "$id:/out/." "$outdir/"
ls -l "$outdir"

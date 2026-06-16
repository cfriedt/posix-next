#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

set -euo pipefail

REALPATH="realpath"
SCRIPT_PATH="$($REALPATH "$(dirname "$0")")"
POSIX_NEXT_PATH="$($REALPATH "$SCRIPT_PATH"/../..)"
DOC_DIR="$POSIX_NEXT_PATH/doc"
BUILD_DIR="$DOC_DIR/_build"
HTML_DIR="$BUILD_DIR/html"

SERVE=0
STRICT=0
LIVE=0

usage() {
	cat <<'EOF'
Usage: docs.sh [OPTIONS]

Build posix-next Sphinx HTML documentation (Doxygen + Sphinx).

Options:
  -h, --help    Show this help and exit
  -s, --serve   Start a local HTTP server after a successful build
  --strict      Treat Sphinx warnings as errors (-W --keep-going, same as CI)
  --live        Run sphinx-autobuild (html-live) instead of a one-shot build

Environment:
  ZEPHYR_BASE          Zephyr tree (auto-sourced from the west workspace when unset)
  SPHINXOPTS           Extra sphinx-build options (default: -j auto -T)
  SPHINXOPTS_EXTRA     Additional Sphinx options (e.g. -t publish)
  DOCS_HTML_BASEURL    Base URL for generated links

Prerequisites:
  pip install -r $ZEPHYR_BASE/doc/requirements.txt
  doxygen graphviz ninja

Output:
  modules/lib/posix/doc/_build/html/

See also:
  doc/getting_started/index.rst — Build documentation locally
EOF
}

while [[ $# -gt 0 ]]; do
	case "$1" in
	-h | --help)
		usage
		exit 0
		;;
	-s | --serve)
		SERVE=1
		shift
		;;
	--strict)
		STRICT=1
		shift
		;;
	--live)
		LIVE=1
		shift
		;;
	*)
		echo "docs.sh: unknown option: $1" >&2
		usage >&2
		exit 2
		;;
	esac
done

if [[ -z "${ZEPHYR_BASE:-}" ]]; then
	WORKSPACE_PATH="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"
	if [[ -f "$WORKSPACE_PATH/zephyr/zephyr-env.sh" ]]; then
		# shellcheck source=/dev/null
		source "$WORKSPACE_PATH/zephyr/zephyr-env.sh"
	fi
fi

if [[ -z "${ZEPHYR_BASE:-}" || ! -d "$ZEPHYR_BASE" ]]; then
	echo "docs.sh: ZEPHYR_BASE is not set or not a directory" >&2
	echo "  source <workspace>/zephyr/zephyr-env.sh" >&2
	exit 1
fi

export ZEPHYR_BASE
export DOCS_HTML_BASEURL="${DOCS_HTML_BASEURL:-https://cfriedt.github.io/posix-next/}"
export SPHINXOPTS="${SPHINXOPTS:--j auto -T}"
export SPHINXOPTS_EXTRA="${SPHINXOPTS_EXTRA:-}"

if [[ "$STRICT" -eq 1 ]]; then
	export SPHINXOPTS="$SPHINXOPTS -W --keep-going"
fi

if [[ "$LIVE" -eq 1 ]]; then
	make -C "$DOC_DIR" html-live
else
	make -C "$DOC_DIR" html
fi

echo "docs.sh: HTML output in $HTML_DIR"

if [[ "$SERVE" -eq 1 ]]; then
	echo "docs.sh: serving at http://127.0.0.1:8000/"
	exec python3 -m http.server -d "$HTML_DIR" --bind 127.0.0.1
fi

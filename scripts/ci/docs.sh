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
REFRESH=1
FETCH=1
TOKEN_FILE=""
HARVEST=""

usage() {
	cat <<'EOF'
Usage: docs.sh [OPTIONS]

Build posix-next Sphinx HTML documentation (Doxygen + Sphinx).

Options:
  -h, --help    Show this help and exit
  -s, --serve   Start a local HTTP server after a successful build
  --strict      Treat Sphinx warnings as errors (-W --keep-going, same as CI)
  --live        Run sphinx-autobuild (html-live) instead of a one-shot build
  --no-refresh  Do not overwrite the fetched twister summary with one
                regenerated from local <workspace>/twister-out* results
  --no-fetch    Do not download the latest CI metrics summaries (twister,
                coverage, asan, ubsan, scan-build) from GitHub workflow
                artifacts into doc/metrics/ before building. Fetching is the
                default, mirroring the Documentation workflow; it requires
                the gh CLI and a token, and fails without them
  -t FILE       Read the GitHub token for fetching from FILE (exported as
                GH_TOKEN). Defaults to ~/.ghtoken; without a token file,
                a pre-set GH_TOKEN is required
  --harvest LIST
                Run local test/analysis suites to (re)generate metrics data
                that is still missing after the fetch, without prompting.
                LIST is 'all' or a comma-separated subset of
                twister,coverage,asan,ubsan,scan-build. Runs take minutes
                (scan-build) to hours (twister, coverage)
  --no-harvest  Never prompt to harvest missing metrics data (the default
                when stdin is not a terminal)

Environment:
  ZEPHYR_BASE          Zephyr tree (auto-sourced from the west workspace when unset)
  GH_TOKEN             GitHub token used for artifact fetching (see -t)
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
	--no-refresh)
		REFRESH=0
		shift
		;;
	--no-fetch)
		FETCH=0
		shift
		;;
	-t | --token-file)
		if [[ $# -lt 2 ]]; then
			echo "docs.sh: $1 requires an argument" >&2
			exit 2
		fi
		TOKEN_FILE="$2"
		shift 2
		;;
	--harvest)
		if [[ $# -lt 2 ]]; then
			echo "docs.sh: --harvest requires an argument" >&2
			exit 2
		fi
		HARVEST="$2"
		shift 2
		;;
	--no-harvest)
		HARVEST="none"
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

# Download the latest metrics summaries from GitHub workflow artifacts,
# mirroring the Documentation workflow's fetch step: newest completed run
# of each producer on main that still has the artifact wins.
fetch_metrics_artifacts() {
	local repo tf id wf artifact found

	if ! command -v gh >/dev/null 2>&1; then
		echo "docs.sh: artifact fetch requires the GitHub CLI (gh);" \
			"use --no-fetch to skip" >&2
		exit 1
	fi

	tf="${TOKEN_FILE:-$HOME/.ghtoken}"
	if [[ -f "$tf" ]]; then
		GH_TOKEN="$(<"$tf")"
		export GH_TOKEN
	elif [[ -n "$TOKEN_FILE" ]]; then
		echo "docs.sh: token file not found: $TOKEN_FILE" >&2
		exit 1
	elif [[ -z "${GH_TOKEN:-}" ]]; then
		echo "docs.sh: no GitHub token: create ~/.ghtoken, pass -t FILE," \
			"or set GH_TOKEN (--no-fetch to skip)" >&2
		exit 1
	fi

	repo="$(git -C "$POSIX_NEXT_PATH" remote get-url origin 2>/dev/null |
		sed -E 's#^(git@github\.com:|https://github\.com/)##; s#\.git$##')"
	repo="${repo:-cfriedt/posix-next}"

	while read -r wf artifact; do
		found=0
		for id in $(gh run list -R "$repo" --workflow "$wf" --branch main \
			--status completed --limit 10 --json databaseId --jq '.[].databaseId'); do
			if gh run download -R "$repo" "$id" --name "$artifact" \
				--dir "$DOC_DIR/metrics" >/dev/null 2>&1; then
				echo "docs.sh: $wf: fetched $artifact (run $id)"
				found=1
				break
			fi
		done
		[[ "$found" -eq 1 ]] ||
			echo "docs.sh: $wf: no $artifact artifact on main; badges will be absent"
	done <<-EOF
		Twister twister-summary
		Coverage coverage-json-snapshot
		ASAN asan-summary
		UBSAN ubsan-summary
		Scan-Build static-analysis-summary
	EOF
}

# Regenerate the twister summary badge metadata from local twister results,
# mirroring the twister-summary-publish CI job. Retry directories
# (twister-out.N) are listed before the final twister-out: the summarizer
# merges last-wins per (suite, platform). doc/metrics/*.json is gitignored;
# CI docs builds fetch the equivalent summaries from workflow artifacts.
refresh_twister_summary() {
	local workspace inputs=()

	workspace="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"
	for f in "$workspace"/twister-out.*/twister.json "$workspace"/twister-out/twister.json; do
		[[ -f "$f" ]] && inputs+=("$f")
	done
	if [[ ${#inputs[@]} -eq 0 ]]; then
		echo "docs.sh: no $workspace/twister-out*/twister.json;" \
			"keeping any existing doc/metrics/twister-summary.json"
		return 0
	fi

	python3 "$SCRIPT_PATH/twister-summarize.py" \
		--output "$DOC_DIR/metrics/twister-summary.json" \
		--commit "$(git -C "$POSIX_NEXT_PATH" rev-parse HEAD)" \
		--profile local \
		"${inputs[@]}"
	echo "docs.sh: refreshed doc/metrics/twister-summary.json (gitignored)"
}

# Offer to run the local test/analysis suites for metrics classes still
# missing after the fetch — the same data the nightlies harvest in CI (see
# runci.sh, coverage.sh, asan.sh, ubsan.sh, scan-build.sh). Prompts appear
# only on a terminal; --harvest LIST runs unattended, --no-harvest silences.
harvest_missing_metrics() {
	local m="$DOC_DIR/metrics" workspace reply
	workspace="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"

	[[ "$HARVEST" != "none" ]] || return 0
	if [[ -z "$HARVEST" && ! -t 0 ]]; then
		return 0
	fi

	want() {
		case ",$HARVEST," in
		*,all,* | *,"$1",*) return 0 ;;
		esac
		[[ -z "$HARVEST" ]] || return 1
		read -r -p "docs.sh: no $2; run $3 now to harvest it ($4)? [y/N] " reply
		[[ "$reply" == [Yy]* ]]
	}

	no_concurrent_twister() {
		if pgrep -af 'twister|runci\.sh' >/dev/null 2>&1; then
			echo "docs.sh: another twister/runci.sh is running; not harvesting" >&2
			return 1
		fi
	}

	if [[ ! -f "$m/twister-summary.json" ]] &&
		want twister "twister summary" "runci.sh" "hours"; then
		no_concurrent_twister || return 0
		"$SCRIPT_PATH/runci.sh"
		refresh_twister_summary
	fi
	if [[ ! -f "$m/coverage-posix.json" ]] &&
		want coverage "coverage data" "coverage.sh" "hours"; then
		no_concurrent_twister || return 0
		COVERAGE_SKIP_UI=1 "$SCRIPT_PATH/coverage.sh"
		cp "$workspace/twister-out/coverage-posix.json" "$m/"
		echo "docs.sh: harvested doc/metrics/coverage-posix.json"
	fi
	if [[ ! -f "$m/asan-summary.json" ]] &&
		want asan "ASAN summary" "asan.sh" "an hour or more"; then
		no_concurrent_twister || return 0
		"$SCRIPT_PATH/asan.sh"
		cp "$workspace/asan-summary.json" "$m/"
		echo "docs.sh: harvested doc/metrics/asan-summary.json"
	fi
	if [[ ! -f "$m/ubsan-summary.json" ]] &&
		want ubsan "UBSAN summary" "ubsan.sh" "an hour or more"; then
		no_concurrent_twister || return 0
		"$SCRIPT_PATH/ubsan.sh"
		cp "$workspace/ubsan-summary.json" "$m/"
		echo "docs.sh: harvested doc/metrics/ubsan-summary.json"
	fi
	if [[ ! -f "$m/static-analysis.json" ]] &&
		want scan-build "static analysis summary" "scan-build.sh" "minutes"; then
		no_concurrent_twister || return 0
		"$SCRIPT_PATH/scan-build.sh"
		cp "$workspace/static-analysis.json" "$m/"
		echo "docs.sh: harvested doc/metrics/static-analysis.json"
	fi
}

if [[ "$FETCH" -eq 1 ]]; then
	fetch_metrics_artifacts
fi
if [[ "$REFRESH" -eq 1 ]]; then
	refresh_twister_summary
fi
harvest_missing_metrics

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

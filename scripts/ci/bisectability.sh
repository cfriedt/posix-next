#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors

# Sampled bisectability check: build and run, at every Nth commit of a range,
# a pseudo-random subset of the test scenarios that commit's own diff touches.
#
# A stop is materialized hermetically: a zephyr worktree at the manifest
# revision of that commit with that commit's patch series applied, worktrees of
# the patched modules, and a worktree of this module at the commit. Twister
# plans the touched test roots there (so platform filters, scenario configs
# and harnesses are honoured), a seeded shuffle keeps a 1/K subset sized to
# the configured number of scenarios per commit, and the subset is built and
# run. Suites a commit touches are found from its diff: tests/posix/<suite>
# and lib/posix/<group> map to the module suite of that name, a patch maps to
# the Zephyr-tree test directories it modifies, and shared code (headers,
# shared/ directories, Kconfig and CMake at the top) draws a seeded sample of
# the configured roots.
#
# Defaults come from a profile of .github/ci-config.json: "bisectability" (a
# quarter of an hour on a feature branch) unless --profile or CI_CONFIG_PROFILE
# names another, such as "bisectability_nightly" (the recent history of main,
# sharded across CI jobs). Runci-style options override the profile and
# anything unknown is passed to twister. Every run draws a fresh seed unless
# --seed is given; the seed picks the scenario subsets and which residue class
# of the stride is visited, and the header line prints it so a run can be
# repeated.
#
# usage: bisectability.sh [options] [twister args...]
#       --profile NAME      ci-config.json profile (default "bisectability",
#                           or $CI_CONFIG_PROFILE)
#   -p, --platform PLAT     platform (repeatable; profile platforms otherwise)
#   -T, --test-root ROOT    candidate test root for sampling, relative to the
#                           workspace or absolute (repeatable; profile roots
#                           otherwise)
#       --only ROOT         plan exactly this root at every stop instead of the
#                           roots the stop's diff touches (repeatable)
#   -r, --range A..B        commit range (default: the profile's "commits" most
#                           recent commits when it has that key, otherwise the
#                           merge-base with origin/main up to HEAD)
#   -s, --stride N          check every Nth commit (profile "stride")
#   -j, --workers N         parallel workers (profile "workers")
#   -n, --scenarios N       scenarios to run per stop (profile
#                           "scenarios_per_commit")
#       --shard I/N         run only the Ith of N contiguous slices of the
#                           stops (for parallel CI jobs sharing one seed)
#       --seed S            seed (profile "seed"; random when unset)
#   -O, --outdir DIR        work and output directory (default build/bisect)
#       --keep              keep the build output of passing stops too
#       --dry-run           list the stops and their roots without building
#   -h, --help

set -euo pipefail

REALPATH="realpath"
PR_DEST="${PR_DEST:-origin/main}"
SCRIPT_PATH="$($REALPATH "$(dirname "$0")")"
POSIX_NEXT_PATH="$($REALPATH "$SCRIPT_PATH"/../..)"
WORKSPACE_PATH="$($REALPATH "$POSIX_NEXT_PATH"/../../..)"
ZEPHYR_REPO="$WORKSPACE_PATH/zephyr"
CI_CONFIG="${CI_CONFIG:-$POSIX_NEXT_PATH/.github/ci-config.json}"
CI_CONFIG_PROFILE="${CI_CONFIG_PROFILE:-bisectability}"
# the profile decides every other default, so it is read ahead of the options
_prev=""
for _a in "$@"; do
  [ "$_prev" = "--profile" ] && CI_CONFIG_PROFILE="$_a"
  _prev="$_a"
done
unset _prev _a

# the Zephyr environment (west, the SDK, twister's Python packages), as runci
# expects it; source the workspace's zephyr-env.sh when it is not set up yet
if ! command -v west >/dev/null 2>&1 && [ -f "$ZEPHYR_REPO/zephyr-env.sh" ]; then
  # shellcheck disable=SC1091
  source "$ZEPHYR_REPO/zephyr-env.sh"
fi

for tool in jq git python3 flock west; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "$tool is required" >&2
    exit 1
  }
done

cfg() {
  jq -r --arg profile "$CI_CONFIG_PROFILE" "$1" "$CI_CONFIG"
}

mapfile -t DEFAULT_PLATFORMS < <(cfg '.[$profile].platforms[]? // empty')
mapfile -t DEFAULT_ROOTS < <(cfg '(.[$profile].roots // .twister_nightly.roots)[]? // empty')
mapfile -t DEFAULT_ARGS < <(cfg '.[$profile].args[]? // empty')
[ "$(cfg '.[$profile] | type')" = "object" ] || {
  echo "no profile \"$CI_CONFIG_PROFILE\" in $CI_CONFIG" >&2
  exit 1
}
STRIDE="$(cfg '.[$profile].stride // 4')"
WORKERS="$(cfg '.[$profile].workers // 4')"
SCENARIOS="$(cfg '.[$profile].scenarios_per_commit // 2')"
SAMPLE_ROOTS="$(cfg '.[$profile].sample_roots // 2')"
COMMITS="$(cfg '.[$profile].commits // 0')"
SEED="$(cfg '.[$profile].seed // empty')"
SHARD="1/1"
RANGE=""
OUTDIR="$POSIX_NEXT_PATH/build/bisect"
KEEP=0
DRY_RUN=0

declare -a PLATFORMS=()
declare -a ROOTS=()
declare -a ONLY=()
declare -a ARGS=()

usage() {
  sed -n '/^# usage:/,/^$/p' "$0" | sed 's/^# \{0,1\}//'
}

need_arg() {
  if [ -z "${2:-}" ]; then
    echo "Error: Missing argument for $1" >&2
    usage >&2
    exit 1
  fi
}

while [ $# -gt 0 ]; do
  case "$1" in
    -h|--help) usage; exit 0 ;;
    --profile) need_arg "$1" "${2:-}"; shift ;;
    -p|--platform) need_arg "$1" "${2:-}"; PLATFORMS+=("$2"); shift ;;
    -T|--test-root) need_arg "$1" "${2:-}"; ROOTS+=("$2"); shift ;;
    --only) need_arg "$1" "${2:-}"; ONLY+=("$2"); shift ;;
    -r|--range) need_arg "$1" "${2:-}"; RANGE="$2"; shift ;;
    -s|--stride) need_arg "$1" "${2:-}"; STRIDE="$2"; shift ;;
    -j|--workers) need_arg "$1" "${2:-}"; WORKERS="$2"; shift ;;
    -n|--scenarios) need_arg "$1" "${2:-}"; SCENARIOS="$2"; shift ;;
    --shard) need_arg "$1" "${2:-}"; SHARD="$2"; shift ;;
    --seed) need_arg "$1" "${2:-}"; SEED="$2"; shift ;;
    -O|--outdir) need_arg "$1" "${2:-}"; OUTDIR="$($REALPATH -m "$2")"; shift ;;
    --keep) KEEP=1 ;;
    --dry-run) DRY_RUN=1 ;;
    --) shift; ARGS+=("$@"); break ;;
    *) ARGS+=("$1") ;;
  esac
  shift
done

[ ${#PLATFORMS[@]} -gt 0 ] || PLATFORMS=("${DEFAULT_PLATFORMS[@]}")
[ ${#ROOTS[@]} -gt 0 ] || ROOTS=("${DEFAULT_ROOTS[@]}")
[ ${#ARGS[@]} -gt 0 ] || ARGS=("${DEFAULT_ARGS[@]}")
[ -n "$SEED" ] || SEED="${SRANDOM:-$RANDOM}"
[[ "$SHARD" =~ ^[1-9][0-9]*/[1-9][0-9]*$ ]] && [ "${SHARD%/*}" -le "${SHARD#*/}" ] || {
  echo "Error: --shard wants I/N with 1 <= I <= N, got $SHARD" >&2
  exit 1
}

cd "$POSIX_NEXT_PATH"
if [ -z "$RANGE" ]; then
  if [ "$COMMITS" -gt 0 ]; then
    # the profile's most recent commits; the whole history when it is shorter
    if git rev-parse -q --verify "HEAD~$COMMITS^{commit}" >/dev/null; then
      RANGE="HEAD~$COMMITS..HEAD"
    else
      RANGE="HEAD"
    fi
  else
    base="$(git merge-base "$PR_DEST" HEAD)"
    RANGE="$base..HEAD"
  fi
fi

# workspace-relative roots: make -T and --only arguments relative when absolute
relroots() {
  local r
  for r in "$@"; do
    if [[ "$r" = /* ]]; then
      r="$($REALPATH -m "$r")"
      r="${r#"$WORKSPACE_PATH"/}"
    fi
    echo "$r"
  done
}
mapfile -t ROOTS < <(relroots "${ROOTS[@]}")
mapfile -t ONLY < <(relroots "${ONLY[@]}")

mkdir -p "$OUTDIR"
PLAN="$OUTDIR/stops.jsonl"
LOCK="$OUTDIR/.lock"

# --- plan: one JSON line per stop with the roots its diff touches -------------
python3 - "$POSIX_NEXT_PATH" "$WORKSPACE_PATH" "$RANGE" "$STRIDE" "$SAMPLE_ROOTS" "$SEED" \
  "$SHARD" "$PLAN" "${#ONLY[@]}" "${ONLY[@]}" "${ROOTS[@]}" <<'PY'
import json, os, random, re, subprocess, sys

repo, workspace, rng, stride, sample_n, seed, shard, plan = sys.argv[1:9]
n_only = int(sys.argv[9])
only, roots = sys.argv[10:10 + n_only], sys.argv[10 + n_only:]
stride, sample_n = int(stride), int(sample_n)
shard_i, shard_n = (int(x) for x in shard.split("/"))
mod_rel = os.path.relpath(repo, workspace)
mod_roots = [r[len(mod_rel) + 1:] for r in roots if r.startswith(mod_rel + "/")]
zephyr_roots = [r[len("zephyr/"):] for r in roots if r.startswith("zephyr/")]


def git(*args, check=True):
    r = subprocess.run(["git", "-C", repo, *args], capture_output=True, text=True)
    if check and r.returncode:
        raise SystemExit(f"git {' '.join(args)} failed: {r.stderr}")
    return r.stdout


def exists(sha, path):
    return subprocess.run(["git", "-C", repo, "cat-file", "-e", f"{sha}:{path}"],
                          capture_output=True).returncode == 0


def suite_dirs(sha):
    """module test suites below the configured module roots that exist at sha"""
    out = []
    for root in mod_roots:
        for d in git("ls-tree", "--name-only", f"{sha}:{root}", check=False).split():
            if exists(sha, f"{root}/{d}/testcase.yaml") or exists(sha, f"{root}/{d}/sample.yaml"):
                out.append(f"{root}/{d}")
    return sorted(out)


SKIP_TOP = ("doc", ".github", "scripts", ".cursor", ".claude", "west.yml")
commits = git("rev-list", "--reverse", rng).split()
# the seed also chooses which residue class of the stride is visited, so runs
# of the same range at different seeds sample different commits
first = random.Random(f"{seed}:offset").randrange(stride) if stride > 1 else 0
stops = []
for i, sha in enumerate(commits, 1):
        if (i - 1 - first) % stride:
            continue
        subject = git("log", "-1", "--format=%s", sha).strip()
        if only:
            stops.append({"pos": i, "sha": sha, "subject": subject, "roots": list(only)})
            continue
        files = git("diff-tree", "--root", "--no-commit-id", "--name-only", "-r", sha).split()
        suites = suite_dirs(sha)
        by_name = {s.rsplit("/", 1)[1]: s for s in suites}
        picked, zdirs, shared = set(), set(), False
        for path in files:
            p = path.split("/")
            if p[0] in SKIP_TOP or path.endswith((".rst", ".md")):
                continue
            hit = next((s for s in suites if path.startswith(s + "/")), None)
            if hit:
                picked.add(hit)
                continue
            if path.startswith("tests/") or path.startswith("samples/"):
                shared = True  # shared test code or a suite outside the roots
                continue
            if path.startswith("lib/posix/"):
                group = p[2] if p[2] != "options" else (p[3] if len(p) > 3 else "")
                if group in by_name:
                    picked.add(by_name[group])
                else:
                    shared = True
                continue
            if path.startswith("zephyr/patches/zephyr/") and path.endswith(".patch"):
                if not exists(sha, path):
                    continue
                body = git("show", f"{sha}:{path}")
                dirs = set(m.group(1) for m in re.finditer(
                    r"^diff --git a/(tests/[^/\s]+/[^/\s]+(?:/[^/\s]+)?)/", body, re.M))
                if dirs:
                    zdirs |= dirs
                else:
                    shared = True
                continue
            shared = True  # include/, Kconfig, CMake, patches.yml, cmake/, ...
        rnd = random.Random(f"{seed}:{sha}")
        if shared:
            pool = [s for s in suites if s not in picked] + [f"zephyr:{z}" for z in zephyr_roots]
            picked |= set(rnd.sample(pool, min(sample_n, len(pool))))
        out = sorted(f"{mod_rel}/{s}" for s in picked if not s.startswith("zephyr:"))
        out += sorted(f"zephyr/{s[len('zephyr:'):]}" for s in picked if s.startswith("zephyr:"))
        out += sorted(f"zephyr/{d}" for d in zdirs)
        if out:
            stops.append({"pos": i, "sha": sha, "subject": subject, "roots": out})
# contiguous slices, like the worker blocks below: neighbouring stops share
# an era more often than not, so a slice keeps its materialized trees longer
with open(plan, "w") as f:
    for stop in stops[(shard_i - 1) * len(stops) // shard_n:shard_i * len(stops) // shard_n]:
        f.write(json.dumps(stop) + "\n")
PY

nstops="$(wc -l < "$PLAN")"
echo "profile $CI_CONFIG_PROFILE, range $RANGE: $(git rev-list --count "$RANGE") commits, stride $STRIDE, shard $SHARD: $nstops stops with suites, seed $SEED"
if [ "$DRY_RUN" -eq 1 ]; then
  jq -r '"\(.pos) \(.sha[0:9]) \(.subject)\n    \(.roots | join("\n    "))"' "$PLAN"
  exit 0
fi

# --- one stop: materialize the era, plan, run the subset ---------------------
locked() {
  flock "$LOCK" "$@"
}

# zephyr worktree at the manifest revision of $sha with its series applied, plus
# worktrees of the modules the series patches; kept as is while the series and
# the manifest are unchanged (key = tree id of zephyr/ + blob id of west.yml),
# otherwise reset to the revision and given the new series: no checkout of a
# fresh worktree, no west update
materialize() {
  local sha="$1" w="$2" key
  key="$(git rev-parse "$sha:zephyr" "$sha:west.yml" | sha1sum | cut -c1-12)"
  if [ -f "$w/era.key" ] && [ "$(cat "$w/era.key")" = "$key" ]; then
    return 0
  fi
  rm -f "$w/era.key"
  python3 - "$POSIX_NEXT_PATH" "$ZEPHYR_REPO" "$WORKSPACE_PATH" "$sha" "$w" "$LOCK" \
    "$(cd "$WORKSPACE_PATH" && west list -f '{name} {url}')" <<'PY'
import os, subprocess, sys, yaml

repo, zrepo, workspace, sha, w, lock, urls = sys.argv[1:8]
urls = dict(line.split(None, 1) for line in urls.splitlines() if " " in line)


def git(*args, cwd=repo, inp=None):
    r = subprocess.run(["git", "-C", cwd, *args], input=inp, capture_output=True)
    if r.returncode:
        raise SystemExit(f"git {' '.join(args)} in {cwd}: {r.stderr.decode()[-500:]}")
    return r.stdout


def locked_git(*args, cwd):
    subprocess.run(["flock", lock, "git", "-C", cwd, "worktree", "prune"], check=True)
    r = subprocess.run(["flock", lock, "git", "-C", cwd, *args], capture_output=True)
    if r.returncode:
        raise SystemExit(f"git {' '.join(args)} in {cwd}: {r.stderr.decode()[-500:]}")
    return r.stdout.decode()


def resolve(src, name, rev):
    """commit id of rev in src; a shallow or narrow clone (CI) fetches just it"""
    r = subprocess.run(["git", "-C", src, "rev-parse", "-q", "--verify", f"{rev}^{{commit}}"],
                       capture_output=True, text=True)
    if r.returncode == 0:
        return r.stdout.strip()
    url = urls.get(name)
    if not url:
        raise SystemExit(f"{name}: {rev} is not in {src} and west knows no url to fetch it from")
    shallow = subprocess.run(["git", "-C", src, "rev-parse", "--is-shallow-repository"],
                             capture_output=True, text=True).stdout.strip() == "true"
    depth = ["--depth=1"] if shallow else []
    locked_git("fetch", "-q", *depth, url, rev, cwd=src)
    return locked_git("rev-parse", "FETCH_HEAD^{commit}", cwd=src).strip()


west = yaml.safe_load(git("show", f"{sha}:west.yml"))
rev = {p["name"]: p.get("revision", "HEAD") for p in west["manifest"]["projects"]}
patches = yaml.safe_load(git("show", f"{sha}:zephyr/patches.yml"))["patches"]
mods = {}
for p in patches:
    mods.setdefault(p.get("module", "zephyr"), []).append(p["path"])
os.makedirs(f"{w}/mods", exist_ok=True)
wanted = set()
for mod, paths in mods.items():
    name = os.path.basename(mod)
    if mod == "zephyr":
        src, tree = zrepo, f"{w}/zephyr"
    else:
        src, tree = os.path.join(workspace, "modules/lib", name), f"{w}/mods/{name}"
    wanted.add(tree)
    commit = resolve(src, name, rev.get(name, "HEAD"))
    if os.path.isdir(os.path.join(tree, ".git")) or os.path.isfile(os.path.join(tree, ".git")):
        # a worktree from the previous era: back to the bare revision in place
        git("reset", "-q", "--hard", commit, cwd=tree)
        git("clean", "-qfdx", cwd=tree)
    else:
        locked_git("worktree", "add", "-q", "--detach", tree, commit, cwd=src)
    for path in paths:
        git("apply", "--whitespace=nowarn", cwd=tree, inp=git("show", f"{sha}:zephyr/patches/{path}"))
    git("add", "-A", cwd=tree)
    git("-c", "user.name=bisectability", "-c", "user.email=bisectability@localhost",
        "commit", "-q", "-m", f"series of {sha[:12]} applied", cwd=tree)
    print(f"  {name}: {len(paths)} patches on {rev.get(name, 'HEAD')}")
# module worktrees the previous era needed and this one does not
for name in os.listdir(f"{w}/mods"):
    tree = f"{w}/mods/{name}"
    if tree not in wanted:
        subprocess.run(["flock", lock, "git", "-C", os.path.join(workspace, "modules/lib", name),
                        "worktree", "remove", "--force", tree], capture_output=True)
PY
  echo "$key" > "$w/era.key"
}

teardown_era() {
  local w="$1" m
  [ -d "$w/zephyr" ] && locked git -C "$ZEPHYR_REPO" worktree remove --force "$w/zephyr" || true
  if [ -d "$w/mods" ]; then
    for m in "$w"/mods/*; do
      [ -d "$m" ] || continue
      locked git -C "$WORKSPACE_PATH/modules/lib/$(basename "$m")" worktree remove --force "$m" || true
    done
  fi
  rm -rf "$w/zephyr" "$w/mods" "$w/era.key"
}

module_list() {
  # the workspace's modules, with the patched ones and this module swapped for
  # their era worktrees
  local w="$1" name path mods=""
  while read -r name path; do
    case "$name" in
      manifest|zephyr) continue ;;
    esac
    if [ -d "$w/mods/$name" ]; then
      path="$w/mods/$name"
    fi
    mods="${mods:+$mods;}$path"
  done < <(cd "$WORKSPACE_PATH" && west list -f '{name} {abspath}')
  echo "${mods:+$mods;}$w/mod"
}

run_stop() {
  local k="$1" pos="$2" sha="$3" subject="$4"
  local w="$OUTDIR/w$k" out roots=() r n K plat=() i
  shift 4
  out="$OUTDIR/stops/$(printf '%04d' "$pos")-${sha:0:9}"
  rm -rf "$out"
  mkdir -p "$out"
  echo "[$(date +%H:%M:%S)] w$k $pos ${sha:0:9} $subject"
  if [ -e "$w/mod/.git" ]; then
    git -C "$w/mod" checkout -q --detach "$sha" && git -C "$w/mod" clean -qfdx
  else
    locked git -C "$POSIX_NEXT_PATH" worktree add -q --detach "$w/mod" "$sha"
  fi
  if ! materialize "$sha" "$w" > "$out/materialize.log" 2>&1; then
    echo "$pos	${sha:0:9}	-	-	error	$(tail -1 "$out/materialize.log")	$subject" >> "$OUTDIR/w$k/results.tsv"
    return 0
  fi
  for r in "$@"; do
    case "$r" in
      zephyr/*) r="$w/${r}" ;;
      *) r="$w/mod/${r#"$(realpath --relative-to="$WORKSPACE_PATH" "$POSIX_NEXT_PATH")"/}" ;;
    esac
    # a patch names files below the suite directory: walk up to the yaml
    while [ "$r" != "$w" ] && [ "$r" != "$w/mod" ] && [ "$r" != "$w/zephyr" ] &&
          [ ! -f "$r/testcase.yaml" ] && [ ! -f "$r/tests.yaml" ] && [ ! -f "$r/sample.yaml" ]; do
      r="$(dirname "$r")"
    done
    if [ -f "$r/testcase.yaml" ] || [ -f "$r/tests.yaml" ] || [ -f "$r/sample.yaml" ]; then
      roots+=(-T "$r")
    fi
  done
  if [ ${#roots[@]} -eq 0 ]; then
    echo "$pos	${sha:0:9}	-	-	skipped	no root exists at this commit	$subject" >> "$OUTDIR/w$k/results.tsv"
    return 0
  fi
  for i in "${PLATFORMS[@]}"; do plat+=(-p "$i"); done
  export ZEPHYR_BASE="$w/zephyr"
  export ZEPHYR_MODULES
  ZEPHYR_MODULES="$(module_list "$w")"
  local twister="$ZEPHYR_BASE/scripts/twister"
  # plan the touched roots, then keep a seeded 1/K subset of about $SCENARIOS
  if ! "$twister" -c -O "$out/twister-out" "${plat[@]}" "${roots[@]}" --dry-run \
        --save-tests "$out/plan.json" "${ARGS[@]}" > "$out/plan.log" 2>&1; then
    echo "$pos	${sha:0:9}	-	-	error	twister planning failed: $(grep -m1 -i 'error' "$out/plan.log" | cut -c1-160)	$subject" >> "$OUTDIR/w$k/results.tsv"
    return 0
  fi
  n="$(jq '[.testsuites[] | select(.status != "filtered" and .status != "skipped")] | length' "$out/plan.json" 2>/dev/null || echo 0)"
  if [ "$n" -eq 0 ]; then
    echo "$pos	${sha:0:9}	-	-	skipped	nothing to build for ${PLATFORMS[*]}	$subject" >> "$OUTDIR/w$k/results.tsv"
    return 0
  fi
  K=$(( (n + SCENARIOS - 1) / SCENARIOS ))
  "$twister" -c -O "$out/twister-out" "${plat[@]}" "${roots[@]}" --load-tests "$out/plan.json" \
    --shuffle-tests --shuffle-tests-seed "$((SEED + pos))" --subset "1/$K" \
    "${ARGS[@]}" > "$out/run.log" 2>&1 || true
  if [ -f "$out/twister-out/twister.json" ]; then
    jq -r --arg pos "$pos" --arg sha "${sha:0:9}" --arg subject "$subject" '
      .testsuites[] | select(.status != "filtered")
      | [$pos, $sha, .name, .platform, .status, (.reason // "" | gsub("\t|\n"; " ") | .[0:160]), $subject]
      | @tsv' "$out/twister-out/twister.json" >> "$OUTDIR/w$k/results.tsv"
  else
    echo "$pos	${sha:0:9}	-	-	error	twister produced no report: $(tail -1 "$out/run.log" | cut -c1-160)	$subject" >> "$OUTDIR/w$k/results.tsv"
  fi
  if [ "$KEEP" -eq 0 ] && ! grep -qE '	(failed|error)	' <(tail -n "$n" "$OUTDIR/w$k/results.tsv"); then
    rm -rf "$out/twister-out"
  fi
}

worker() {
  # worker k walks the kth contiguous block of the stops, so the workers sit in
  # different parts of the history and each reuses its era across neighbours
  local k="$1" i=0 pos sha subject roots lo hi
  lo=$(( k * nstops / WORKERS ))
  hi=$(( (k + 1) * nstops / WORKERS ))
  : > "$OUTDIR/w$k/results.tsv"
  while IFS= read -r line; do
    if [ "$i" -ge "$lo" ] && [ "$i" -lt "$hi" ]; then
      pos="$(jq -r .pos <<< "$line")"
      sha="$(jq -r .sha <<< "$line")"
      subject="$(jq -r .subject <<< "$line")"
      mapfile -t roots < <(jq -r '.roots[]' <<< "$line")
      run_stop "$k" "$pos" "$sha" "$subject" "${roots[@]}"
    fi
    i=$((i + 1))
  done < "$PLAN"
  locked git -C "$POSIX_NEXT_PATH" worktree remove --force "$OUTDIR/w$k/mod" 2>/dev/null || true
  teardown_era "$OUTDIR/w$k"
}

rm -rf "$OUTDIR/stops"
mkdir -p "$OUTDIR/stops"
pids=()
for ((k = 0; k < WORKERS; k++)); do
  mkdir -p "$OUTDIR/w$k"
  worker "$k" &
  pids+=($!)
done
status=0
for pid in "${pids[@]}"; do
  wait "$pid" || status=1
done

# --- report --------------------------------------------------------------------
cat "$OUTDIR"/w*/results.tsv | sort -n -k1,1 > "$OUTDIR/results.tsv"
total="$(wc -l < "$OUTDIR/results.tsv")"
bad="$(awk -F'\t' '$5 == "failed" || $5 == "error"' "$OUTDIR/results.tsv")"
jq -n --arg profile "$CI_CONFIG_PROFILE" --arg range "$RANGE" --argjson seed "$SEED" \
  --argjson stride "$STRIDE" --arg shard "$SHARD" --argjson stops "$nstops" --argjson results "$total" \
  --argjson failed "$(printf '%s' "$bad" | grep -c . || true)" \
  '{profile: $profile, range: $range, seed: $seed, stride: $stride, shard: $shard,
    stops: $stops, results: $results, failed: $failed}' > "$OUTDIR/summary.json"
echo
echo "$total scenario results over $nstops stops ($OUTDIR/results.tsv)"
if [ -n "$bad" ]; then
  echo "FAILURES:"
  printf '%s\n' "$bad" | awk -F'\t' '{ printf "  %s %s %s: %s %s (%s) %s\n", $1, $2, $7, $3, $4, $5, $6 }'
  status=1
else
  echo "no failures"
fi
exit $status

# action-zephyr-setup (posix fork)

Internal fork of [zephyrproject-rtos/action-zephyr-setup](https://github.com/zephyrproject-rtos/action-zephyr-setup) at **v1.0.9**, extended for posix-next CI.

## Changes vs upstream

- **`cache-west-projects`** (default `true`): After a cache miss, runs `west init` + `west update`, then archives all west project paths (from `west list`, plus `.west/`) into `.west-workspace-cache.tar.zst`. The manifest `app-path` is never included. On a cache hit, the tarball is extracted into the workspace and **`west update` is skipped** — trees are already at the revisions pinned by `west.yml`.
- **Cache key**: `west-projects-v2-${{ hashFiles('…/west.yml', 'west.yml') }}` — **exact-match only, no `restore-keys` prefix fallback**. The `west.yml` hash pins the manifest (and thus the Zephyr revision), so a same-`west.yml` cache is still shared across branches via the exact key (default-branch caches are visible to all branches). A bare `west-projects-` prefix fallback was **removed**: across a Zephyr version bump it restored a stale, different-revision workspace, `west update` (skipped on a cache "hit") never re-homed it, and the stale tree was saved back under the new key — poisoning it so every later run got a v4.3.0 tree under the v4.4.1 key. The `v2-` salt evicts any already-poisoned v1 caches.
- **`enable-ccache`** default **`false`**: posix workflows use workspace `${{ github.workspace }}/.ccache` with explicit restore/save steps.
- **Nested `manifest.self.path`**: `west init -l` cannot place `.west` at the workspace root when `self.path` is nested (e.g. `modules/lib/posix`). Bootstrap writes `.west/config` at the workspace root instead. [setup-workspace](../setup-workspace/action.yml) re-homes the GitHub checkout into `modules/lib/posix/` first.
- Sets **`ZEPHYR_BASE`** and **`ZEPHYR_SDK_INSTALL_DIR`** in `GITHUB_ENV` after SDK detection.

Patches are applied after this action (see [setup-workspace](../setup-workspace/action.yml)); the tarball is always **pre-patch**.

## Usage

Callers must checkout the manifest before this action.

```yaml
- uses: actions/checkout@v5
  with:
    path: modules/lib/posix
    fetch-depth: 0

- uses: ./.github/actions/action-zephyr-setup
  with:
    base-path: .
    app-path: .
    toolchains: arm-zephyr-eabi:riscv64-zephyr-elf:x86_64-zephyr-elf
    enable-ccache: false
```

## Cache timing

The job step summary reports `cache-hit` for the west workspace tarball. Compare setup time when `west.yml` is unchanged (hit) vs changed (miss).

## Upstream

Consider contributing tarball-based `cache-west-projects` to zephyrproject-rtos/action-zephyr-setup if the behavior proves stable across manifests.

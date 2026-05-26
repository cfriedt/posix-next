# action-zephyr-setup (posix fork)

Internal fork of [zephyrproject-rtos/action-zephyr-setup](https://github.com/zephyrproject-rtos/action-zephyr-setup) at **v1.0.9**, extended for posix-next CI.

## Changes vs upstream

- **`cache-west-projects`** (default `true`): After a cache miss, runs `west init` + `west update`, then archives all west project paths (from `west list`, plus `.west/`) into `.west-workspace-cache.tar.zst`. The manifest `app-path` is never included. On a cache hit, the tarball is extracted into the workspace and **`west update` is skipped** — trees are already at the revisions pinned by `west.yml`.
- **Cache key**: `west-projects-${{ hashFiles('…/west.yml', 'west.yml') }}` with **`restore-keys`** prefix `west-projects-`. Exact hits require the same `west.yml` hash; prefix fallback lets scheduled/`main` jobs reuse a cache created on another branch (e.g. Twister on a PR). Caches on the default branch are visible to all branches; feature-branch caches are not visible to `schedule` on `main`.
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

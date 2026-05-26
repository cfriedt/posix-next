# setup-workspace

Composite action for posix-next CI: re-home manifest, forked [action-zephyr-setup](../action-zephyr-setup/README.md), `west patch apply`, and **restore** workspace ccache.

Pair with [save-workspace-ccache](../save-workspace-ccache/) at the end of the job (after Twister).

## Ccache

- **`action-zephyr-setup`** keeps `enable-ccache: false` (upstream-style `$HOME/.cache/ccache` + timestamp keys).
- This action restores `${{ github.workspace }}/.ccache` for Twister `-c` and exports `CCACHE_DIR` / `CCACHE_BASEDIR` / `CCACHE_MAXSIZE` / `CCACHE_COMPILERCHECK`.
- Actions cache key: `ccache-<ccache-key-prefix>-<OS>-<SDK_VERSION hash>-<run_id>` with `restore-keys` prefix fallback.
- **Coverage workflow**: set `cache-ccache: false` on the merge job; shards restore, merge saves via `ccache-save-key` output.
- Saving is **not** done here (tests must run first); call `save-workspace-ccache` last.

## Defaults

- **`manifest-path`**: `modules/lib/posix` (matches `manifest.self.path` in `west.yml`)
- Requires a prior **`actions/checkout`** step.
- **`ccache-key-prefix`**: `twister` (use `coverage` in the coverage workflow)

## Example

```yaml
- uses: actions/checkout@v5
  with:
    fetch-depth: 0

- uses: ./.github/actions/setup-workspace
  id: workspace
  with:
    toolchains: arm-zephyr-eabi:riscv64-zephyr-elf:x86_64-zephyr-elf
    ccache-key-prefix: twister

# ... run twister ...

- uses: ./.github/actions/save-workspace-ccache
  if: always() && !cancelled()
  with:
    cache-primary-key: ${{ steps.workspace.outputs.ccache-primary-key }}
    save: ${{ github.event_name == 'push' || github.event_name == 'schedule' }}
```

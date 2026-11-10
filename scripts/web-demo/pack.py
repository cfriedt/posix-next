#!/usr/bin/env python3

# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors

"""Bundle a Zephyr QEMU build with the browser QEMU for the in-browser demo.

The bundle is a flat directory the docs page loads relative to itself:
the emulator (qemu-system-<target>.js / .wasm / .worker.js), the terminal
runtime, the Zephyr image and a manifest.json that names them and carries the
QEMU command line. The command line is the one the Zephyr build's ``run``
target uses, minus what only makes sense on a host (the pid file, the stdio
chardev mux and monitor, icount and the rtc clock) and with the image at the
path it has inside the emulator's virtual file system.

Large files are gzipped (the page inflates them): GitHub Pages does not
compress WebAssembly, and the emulator is tens of megabytes.
"""

import argparse
import datetime
import gzip
import json
import pathlib
import re
import shlex
import shutil
import sys

GZIP_MIN_SIZE = 1 << 20
# -opt value pairs that only make sense for a host QEMU
HOST_ONLY_OPTS = {"-pidfile", "-chardev", "-serial", "-mon", "-icount", "-rtc"}
GUEST_IMAGE = "/zephyr.elf"
# the browser QEMU predates the bare rv64i/rv32i models; the generic models
# take the same feature list on top of their defaults
BARE_CPU_FALLBACK = {"rv64i": "rv64", "rv32i": "rv32"}


def run_command(build_dir: pathlib.Path) -> list[str]:
    """The QEMU command line of the build's ``run`` target."""
    ninja = (build_dir / "build.ninja").read_text()
    m = re.search(r"(qemu-system-\S+ .*?-kernel \S+)", ninja)
    if m is None:
        sys.exit(f"{build_dir}: no QEMU run command in build.ninja")
    return shlex.split(m.group(1))


def browser_args(command: list[str]) -> list[str]:
    args = []
    it = iter(command[1:])
    for opt in it:
        if opt in HOST_ONLY_OPTS:
            next(it)
        elif opt == "-kernel":
            next(it)
            args += ["-kernel", GUEST_IMAGE]
        elif opt == "-cpu":
            model, _, features = next(it).partition(",")
            model = BARE_CPU_FALLBACK.get(model, model)
            args += ["-cpu", ",".join(filter(None, (model, features)))]
        else:
            args.append(opt)
    if "-nographic" not in args:
        args.append("-nographic")
    return args


def config_value(build_dir: pathlib.Path, name: str) -> str:
    for line in (build_dir / "zephyr" / ".config").read_text().splitlines():
        if line.startswith(f"CONFIG_{name}="):
            return line.split("=", 1)[1].strip('"')
    return ""


def kernel_version(build_dir: pathlib.Path) -> str:
    header = build_dir / "zephyr" / "include" / "generated" / "zephyr" / "version.h"
    m = re.search(r'KERNEL_VERSION_STRING\s+"([^"]+)"', header.read_text())
    return m.group(1) if m else ""


def install(
    src: pathlib.Path, outdir: pathlib.Path, name: str | None = None
) -> tuple[str, int]:
    """Copy src into outdir, gzipped when large; returns (bundle name, raw size)."""
    name = name or src.name
    size = src.stat().st_size
    if size < GZIP_MIN_SIZE:
        shutil.copyfile(src, outdir / name)
        return name, size
    with (
        open(src, "rb") as f,
        gzip.GzipFile(outdir / f"{name}.gz", "wb", 9, mtime=0) as g,
    ):
        shutil.copyfileobj(f, g)
    return f"{name}.gz", size


def main() -> None:
    ap = argparse.ArgumentParser(
        description=__doc__.splitlines()[0], allow_abbrev=False
    )
    ap.add_argument(
        "-q",
        "--qemu",
        required=True,
        type=pathlib.Path,
        help="browser QEMU build (output of build-qemu-wasm.sh)",
    )
    ap.add_argument(
        "-b",
        "--build",
        required=True,
        type=pathlib.Path,
        help="Zephyr build directory of the image",
    )
    ap.add_argument(
        "-o", "--out", required=True, type=pathlib.Path, help="bundle directory"
    )
    ap.add_argument(
        "--sample", default="", help="source path of the application, for the manifest"
    )
    ap.add_argument(
        "--commit", default="", help="posix-next revision, for the manifest"
    )
    ap.add_argument("--qemu-source", default="", help="URL of the browser QEMU sources")
    args = ap.parse_args()

    command = run_command(args.build)
    target = command[0].removeprefix("qemu-system-")
    qemu_js = args.qemu / f"qemu-system-{target}.js"
    if not qemu_js.exists():
        sys.exit(f"{args.qemu}: no {qemu_js.name}; the image needs a {target} emulator")

    args.out.mkdir(parents=True, exist_ok=True)
    files = {}
    sizes = {}
    for key, name in {
        "js": qemu_js.name,
        "wasm": f"qemu-system-{target}.wasm",
        "worker": f"qemu-system-{target}.worker.js",
        "xterm": "xterm.js",
        "xterm_css": "xterm.css",
        "xterm_fit": "xterm-addon-fit.js",
        "xterm_pty": "xterm-pty.js",
        "qemu_license": "QEMU-COPYING",
    }.items():
        files[key], sizes[key] = install(args.qemu / name, args.out)
    files["image"], sizes["image"] = install(
        args.build / "zephyr" / "zephyr.elf", args.out
    )

    manifest = {
        "qemu": {
            "target": target,
            "source": args.qemu_source,
            "files": {k: files[k] for k in ("js", "wasm", "worker", "qemu_license")},
        },
        "terminal": {
            k: files[k] for k in ("xterm", "xterm_css", "xterm_fit", "xterm_pty")
        },
        "image": {
            "file": files["image"],
            "board": config_value(args.build, "BOARD_TARGET"),
            "sample": args.sample,
            "zephyr": kernel_version(args.build),
            "commit": args.commit,
        },
        "sizes": sizes,
        "args": browser_args(command),
        "built": datetime.datetime.now(datetime.timezone.utc).isoformat(
            timespec="seconds"
        ),
    }
    (args.out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()

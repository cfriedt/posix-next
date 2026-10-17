.. SPDX-License-Identifier: Apache-2.0
.. SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors

Running Zephyr in the browser
#############################

The documentation page of the :zephyr:code-sample:`posix-toybox` sample runs the sample inside
the reader's browser. Zephyr has no WebAssembly architecture, and a shell with processes needs a
real CPU with an MMU anyway, so the browser runs the *emulator*: QEMU compiled to WebAssembly by
the `qemu-wasm`_ fork, booting the ``qemu_riscv64`` image of the sample built without its command
tour, so that it goes straight to the shell. The guest console is an `xterm.js`_ terminal wired to
the emulator's standard I/O through `xterm-pty`_.

The pieces here build that bundle:

``build-qemu-wasm.sh``
   Checks out the pinned qemu-wasm revision, builds its emscripten environment image, builds
   ``qemu-system-riscv64`` with ``Dockerfile`` and extracts the emulator and the terminal runtime
   into a directory. Needs docker with BuildKit, about an hour of CPU and a few gigabytes of disk.

``qemu-wasm-image.sh``
   Prints the content-addressed tag of that build. The ``Web Demo`` workflow publishes the build
   as ``ghcr.io/<repository>/qemu-wasm:<tag>`` and reuses it until the inputs change;
   ``extract-qemu-wasm.sh`` unpacks a published image.

``pack.py``
   Bundles a Zephyr build directory with the emulator: the image, the emulator, the terminal and
   a ``manifest.json`` naming them. The QEMU command line in the manifest is the one the build's
   ``run`` target uses, minus the host-only parts (pid file, stdio multiplexer, monitor, icount).
   Large files are gzipped because GitHub Pages does not compress WebAssembly.

The page side lives in the documentation tree. The ``.. web-demo::`` directive
(``doc/extensions/posix_web_demo``) puts the demo on a page; ``doc/_static/web-demo.js`` and
``web-demo.css`` draw the poster, download and inflate the bundle, start the emulator and offer to
fill the tab; ``doc/_root/web-demo-sw.js`` is a service worker, served from the site root, that
adds the ``Cross-Origin-Opener-Policy`` and ``Cross-Origin-Embedder-Policy`` headers. QEMU's
threads are web workers sharing memory, which browsers allow only on pages with those headers,
and GitHub Pages cannot set them itself. The first click on the poster registers the worker, with
the page's directory as its scope so no other page is affected, and reloads the page once.

Building the bundle by hand
***************************

.. code-block:: console

   $ west build -p -b qemu_riscv64 -d build/toybox-qemu_riscv64 samples/posix/toybox \
       -- -DCONFIG_SAMPLE_TOYBOX_TOUR=n
   $ scripts/web-demo/build-qemu-wasm.sh -o /tmp/qemu-wasm
   $ scripts/web-demo/pack.py -q /tmp/qemu-wasm -b build/toybox-qemu_riscv64 -o doc/_static/demo
   $ make -C doc html

The documentation build ships whatever ``doc/_static/demo`` holds; without it the demo page
shows the poster and says the bundle is missing. The ``Documentation`` workflow fetches the newest
``web-demo`` artifact of the ``Web Demo`` workflow instead of building it. Any ``qemu_riscv64``
image can be packed the same way; the manifest carries its command line.

Serve the result over HTTP (``python3 -m http.server`` in ``doc/_build/html`` will do): service
workers and WebAssembly threads are not available from ``file://`` pages.

.. _qemu-wasm: https://github.com/ktock/qemu-wasm
.. _xterm.js: https://xtermjs.org/
.. _xterm-pty: https://github.com/mame/xterm-pty

/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 */

/*
 * The in-browser demo a `.. web-demo::` directive puts on a page: a Zephyr
 * image running under QEMU compiled to WebAssembly, in a terminal that
 * replaces the poster when it is played. The bundle (manifest.json and the
 * files it names) comes from scripts/web-demo/pack.py; see
 * scripts/web-demo/README.rst.
 */
(() => {
	const root = document.querySelector(".pn-demo");

	if (root === null) {
		return;
	}

	const base = new URL(root.dataset.base, document.baseURI);
	const frame = root.querySelector(".pn-demo-frame");
	const play = root.querySelector(".pn-demo-play");
	const status = root.querySelector(".pn-demo-status");
	const termEl = root.querySelector(".pn-demo-term");
	const controls = root.querySelector(".pn-demo-controls");
	const maxBtn = root.querySelector(".pn-demo-max");
	const closeBtn = root.querySelector(".pn-demo-close");
	/* the page reloads itself with this query to be served isolated, then plays */
	const DEMO = "demo";
	let manifest = null;

	const url = (name) => new URL(name, base).href;

	const say = (text) => {
		status.hidden = text === "";
		status.textContent = text;
	};

	const fail = (text) => {
		say(text);
		play.hidden = true;
	};

	const loadScript = (src) => new Promise((resolve, reject) => {
		const s = document.createElement("script");

		s.src = src;
		s.onload = resolve;
		s.onerror = () => reject(new Error(`failed to load ${src}`));
		document.head.appendChild(s);
	});

	const loadStyle = (href) => {
		const l = document.createElement("link");

		l.rel = "stylesheet";
		l.href = href;
		document.head.appendChild(l);
	};

	const megabytes = (n) => `${(n / 1048576).toFixed(1)} MB`;

	/* fetch a bundle file, inflating it when the packer gzipped it */
	const download = async (name, label) => {
		const response = await fetch(url(name));

		if (!response.ok) {
			throw new Error(`${name}: HTTP ${response.status}`);
		}
		const total = Number(response.headers.get("content-length")) || 0;
		let got = 0;
		const progress = new TransformStream({
			transform(chunk, controller) {
				got += chunk.byteLength;
				say(`Downloading ${label}: ${megabytes(got)}${total ? ` of ${megabytes(total)}` : ""}`);
				controller.enqueue(chunk);
			},
		});
		let stream = response.body.pipeThrough(progress);

		if (name.endsWith(".gz")) {
			stream = stream.pipeThrough(new DecompressionStream("gzip"));
		}
		return new Uint8Array(await new Response(stream).arrayBuffer());
	};

	/*
	 * SharedArrayBuffer needs a cross-origin isolated page. The service worker
	 * at the site root adds the headers to navigations that ask for them with
	 * the demo query, so no other page is touched; registering it takes one
	 * reload, after which the demo resumes on its own.
	 */
	const isolate = async () => {
		if (window.crossOriginIsolated) {
			return true;
		}
		if (!window.isSecureContext) {
			throw new Error("the page must be served over HTTPS or from localhost");
		}
		const target = new URL(location.href);

		/* a page the worker already serves without isolation is beyond its help */
		if (!("serviceWorker" in navigator) ||
		    (navigator.serviceWorker.controller !== null && target.searchParams.has(DEMO))) {
			return false;
		}
		say("Preparing the page for the emulator\u2026");
		await navigator.serviceWorker.register(new URL(root.dataset.sw, document.baseURI));
		await navigator.serviceWorker.ready;
		target.searchParams.set(DEMO, "");
		location.replace(target);
		return new Promise(() => {});
	};

	const start = async () => {
		play.hidden = true;
		try {
			if (!(await isolate())) {
				fail("This browser does not allow the emulator's shared memory here. " +
				     "Try a current Chrome, Firefox, Edge or Safari.");
				return;
			}
			say("Loading the terminal…");
			loadStyle(url(manifest.terminal.xterm_css));
			await loadScript(url(manifest.terminal.xterm));
			await Promise.all([
				loadScript(url(manifest.terminal.xterm_fit)),
				loadScript(url(manifest.terminal.xterm_pty)),
			]);

			const wasm = await download(manifest.qemu.files.wasm, "the emulator");
			const image = await download(manifest.image.file, "Zephyr");

			say("Compiling the emulator…");
			const wasmModule = await WebAssembly.compile(wasm);
			const qemu = await import(url(manifest.qemu.files.js));

			const term = new Terminal({
				cursorBlink: true,
				fontSize: 14,
				fontFamily: "'DejaVu Sans Mono', 'Liberation Mono', Menlo, Consolas, monospace",
				scrollback: 5000,
				theme: {
					background: "#1b1f24",
					foreground: "#e6e6e6",
					cursor: "#e6e6e6",
				},
			});
			const fit = new FitAddon.FitAddon();
			const { master, slave } = openpty();

			term.loadAddon(fit);
			term.loadAddon(master);
			termEl.hidden = false;
			term.open(termEl);
			fit.fit();
			new ResizeObserver(() => fit.fit()).observe(termEl);
			controls.hidden = false;

			const Module = {
				arguments: manifest.args,
				pty: slave,
				mainScriptUrlOrBlob: url(manifest.qemu.files.js),
				locateFile: (name) => url(name),
				instantiateWasm: (imports, done) => {
					WebAssembly.instantiate(wasmModule, imports).then((instance) => {
						done(instance, wasmModule);
					});
					return {};
				},
				preRun: [
					(mod) => {
						mod.FS.createDataFile("/", "zephyr.elf", image, true, false, false);
					},
				],
				print: (text) => console.log(text),
				printErr: (text) => console.warn(text),
				onExit: () => {
					say("The emulator stopped. Reload the page to run it again.");
					frame.classList.add("pn-demo--stopped");
				},
			};

			say("");
			root.classList.add("pn-demo--running");
			term.focus();
			await qemu.default(Module);
			/* the pty serves poll(): report what it has instead of blocking */
			const oldPoll = Module.TTY.stream_ops.poll;

			Module.TTY.stream_ops.poll = (stream, timeout) => {
				if (!slave.readable) {
					return (slave.readable ? 1 : 0) | (slave.writable ? 4 : 0);
				}
				return oldPoll.call(stream, timeout);
			};
		} catch (err) {
			console.error(err);
			fail(`The demo could not start: ${err.message}`);
		}
	};

	const setMaximized = (on) => {
		frame.classList.toggle("pn-demo--max", on);
		document.body.classList.toggle("pn-demo-body-max", on);
	};

	maxBtn.addEventListener("click", () => setMaximized(true));
	closeBtn.addEventListener("click", () => setMaximized(false));
	document.addEventListener("keydown", (event) => {
		if (event.key === "Escape" && frame.classList.contains("pn-demo--max") &&
		    !termEl.contains(document.activeElement)) {
			setMaximized(false);
		}
	});
	play.addEventListener("click", start);

	fetch(url("manifest.json"))
		.then((response) => (response.ok ? response.json() : null))
		.then((json) => {
			if (json === null) {
				fail("The emulator bundle is not part of this documentation build.");
				return;
			}
			manifest = json;
			if (typeof DecompressionStream === "undefined" || typeof WebAssembly === "undefined") {
				fail("This browser is too old for the emulator.");
				return;
			}
			if (new URL(location.href).searchParams.has(DEMO)) {
				start();
			}
		})
		.catch(() => fail("The emulator bundle is not part of this documentation build."));
})();

/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 */

/*
 * The in-browser emulator runs QEMU's threads as web workers over a
 * SharedArrayBuffer, which browsers hand out only to cross-origin isolated
 * pages: ones served with the Cross-Origin-Opener-Policy and
 * Cross-Origin-Embedder-Policy headers, whose workers and subresources pass
 * the same policy. GitHub Pages sets no custom headers, so this service
 * worker adds them: to navigations that ask for them with the demo query
 * (a demo page reloads itself that way, so the rest of the site keeps
 * loading as before) and to every same-origin subresource it serves. It
 * lives at the site root because a worker only controls pages under its
 * own path, and the bundle lives under _static.
 */

const DEMO = "demo";

self.addEventListener("install", () => self.skipWaiting());
self.addEventListener("activate", (event) => event.waitUntil(self.clients.claim()));

const isolated = (response) => {
	if (response.status === 0) {
		return response;
	}
	const headers = new Headers(response.headers);

	headers.set("Cross-Origin-Opener-Policy", "same-origin");
	headers.set("Cross-Origin-Embedder-Policy", "require-corp");
	headers.set("Cross-Origin-Resource-Policy", "same-origin");
	return new Response(response.body, {
		status: response.status,
		statusText: response.statusText,
		headers,
	});
};

self.addEventListener("fetch", (event) => {
	const request = event.request;
	const url = new URL(request.url);

	if (url.origin !== self.location.origin) {
		return;
	}
	if (request.mode === "navigate" && !url.searchParams.has(DEMO)) {
		return;
	}
	if (request.cache === "only-if-cached" && request.mode !== "same-origin") {
		return;
	}
	event.respondWith(fetch(request).then(isolated));
});

# syntax=docker/dockerfile:1.5
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors

# QEMU system emulator compiled to WebAssembly, from the qemu-wasm fork of
# QEMU (https://github.com/ktock/qemu-wasm), plus the terminal runtime the
# browser page needs. The build context is the qemu-wasm checkout and
# BASE_IMAGE is the emscripten environment built from its own Dockerfile;
# see build-qemu-wasm.sh. The final stage carries only the outputs, so it
# is small enough to publish as the build cache.

ARG BASE_IMAGE=buildqemu

FROM ${BASE_IMAGE} AS build
ARG TARGET=riscv64
# wasm linear memory: guest RAM (-m) plus the translation cache and QEMU's own heap
ARG TOTAL_MEMORY=1024MB
WORKDIR /build
RUN --mount=type=bind,source=.,target=/qemu,rw \
    EXTRA_CFLAGS="-O3 -Wno-error=unused-command-line-argument -matomics -mbulk-memory \
      -DNDEBUG -DG_DISABLE_ASSERT -D_GNU_SOURCE -sASYNCIFY=1 -pthread -sPROXY_TO_PTHREAD=1 \
      -sFORCE_FILESYSTEM -sALLOW_TABLE_GROWTH -sTOTAL_MEMORY=${TOTAL_MEMORY} -sWASM_BIGINT \
      -sMALLOC=mimalloc --js-library=/build/node_modules/xterm-pty/emscripten-pty.js \
      -sEXPORT_ES6=1 -sASYNCIFY_IMPORTS=ffi_call_js" && \
    emconfigure /qemu/configure --static --target-list=${TARGET}-softmmu --cpu=wasm32 \
      --cross-prefix= --without-default-features --enable-system --with-coroutine=fiber \
      --extra-cflags="${EXTRA_CFLAGS}" --extra-cxxflags="${EXTRA_CFLAGS}" \
      --extra-ldflags="-sEXPORTED_RUNTIME_METHODS=getTempRet0,setTempRet0,addFunction,removeFunction,TTY,FS" && \
    emmake make -j "$(nproc)" qemu-system-${TARGET} && \
    cp /qemu/COPYING QEMU-COPYING
# the terminal: xterm.js renders it, xterm-pty wires it to the emulator's stdio
RUN npm i --no-audit --no-fund @xterm/xterm@5.5.0 @xterm/addon-fit@0.10.0 && \
    mkdir -p /out && \
    cp qemu-system-${TARGET} /out/qemu-system-${TARGET}.js && \
    cp qemu-system-${TARGET}.wasm qemu-system-${TARGET}.worker.js /out/ && \
    cp node_modules/xterm-pty/index.js /out/xterm-pty.js && \
    cp node_modules/@xterm/xterm/lib/xterm.js node_modules/@xterm/xterm/css/xterm.css /out/ && \
    cp node_modules/@xterm/addon-fit/lib/addon-fit.js /out/xterm-addon-fit.js && \
    cp QEMU-COPYING /out/

FROM scratch AS out
COPY --from=build /out/ /out/
# docker create needs a command even though it never runs
CMD ["/none"]

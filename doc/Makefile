# SPDX-License-Identifier: Apache-2.0
# Makefile for posix-next documentation build.

BUILDDIR    ?= _build
# resolve once through the shell: GNU make 4.3 spawns a bare command via its
# own PATH walk, which stops at $ZEPHYR_BASE/scripts/cmake/ (a directory)
CMAKE       ?= $(or $(shell command -v cmake),cmake)
DOC_TAG     ?= development
SPHINXOPTS  ?= -j auto -T
SPHINXOPTS_EXTRA ?=

.PHONY: html html-fast html-live clean

html: configure
	$(CMAKE) --build $(BUILDDIR) --target html

html-fast: configure
	$(CMAKE) --build $(BUILDDIR) --target html

html-live: configure
	$(CMAKE) --build $(BUILDDIR) --target html-live

configure:
	$(CMAKE) \
		-GNinja \
		-B$(BUILDDIR) \
		-S. \
		-DDOC_TAG=$(DOC_TAG) \
		-DSPHINXOPTS="$(SPHINXOPTS)" \
		-DSPHINXOPTS_EXTRA="$(SPHINXOPTS_EXTRA)"

clean:
	$(CMAKE) --build $(BUILDDIR) --target clean

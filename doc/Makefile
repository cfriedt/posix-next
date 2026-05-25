# SPDX-License-Identifier: Apache-2.0
# Makefile for posix-next documentation build.

BUILDDIR    ?= _build
DOC_TAG     ?= development
SPHINXOPTS  ?= -j auto -T
SPHINXOPTS_EXTRA ?=

.PHONY: html html-fast html-live clean

html: configure
	cmake --build $(BUILDDIR) --target html

html-fast: configure
	cmake --build $(BUILDDIR) --target html

html-live: configure
	cmake --build $(BUILDDIR) --target html-live

configure:
	cmake \
		-GNinja \
		-B$(BUILDDIR) \
		-S. \
		-DDOC_TAG=$(DOC_TAG) \
		-DSPHINXOPTS="$(SPHINXOPTS)" \
		-DSPHINXOPTS_EXTRA="$(SPHINXOPTS_EXTRA)"

clean:
	cmake --build $(BUILDDIR) --target clean

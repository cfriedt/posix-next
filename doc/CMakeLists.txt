# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
project(posix-next-doc LANGUAGES)

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

# ZEPHYR_BASE: path to the Zephyr v4.3.0 tree (sibling of this module in the
# west workspace, or overridden by the environment variable).
if(NOT DEFINED ENV{ZEPHYR_BASE})
  get_filename_component(ZEPHYR_BASE "${CMAKE_CURRENT_LIST_DIR}/../../../zephyr" ABSOLUTE)
else()
  set(ZEPHYR_BASE $ENV{ZEPHYR_BASE})
endif()
message(STATUS "ZEPHYR_BASE: ${ZEPHYR_BASE}")

set(DOCS_CFG_DIR    ${CMAKE_CURRENT_LIST_DIR})
set(DOCS_SRC_DIR    ${CMAKE_CURRENT_LIST_DIR})
set(DOCS_BUILD_DIR  ${CMAKE_CURRENT_BINARY_DIR})
set(DOCS_HTML_DIR   ${CMAKE_CURRENT_BINARY_DIR}/html)
set(DOCS_DOCTREE_DIR ${CMAKE_CURRENT_BINARY_DIR}/doctrees)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------

set(SPHINXOPTS       "-j auto -T" CACHE STRING "Sphinx build options")
set(SPHINXOPTS_EXTRA ""            CACHE STRING "Extra Sphinx options")
set(DOC_TAG          "development" CACHE STRING "Doc tag (development|release)")

separate_arguments(SPHINXOPTS)
separate_arguments(SPHINXOPTS_EXTRA)

# ---------------------------------------------------------------------------
# Tools
# ---------------------------------------------------------------------------

find_program(SPHINXBUILD sphinx-build)
if(NOT SPHINXBUILD)
  message(FATAL_ERROR "'sphinx-build' not found. Install it with: pip install -r ${ZEPHYR_BASE}/doc/requirements.txt")
endif()

find_program(DOXYGEN_EXECUTABLE doxygen)
if(NOT DOXYGEN_EXECUTABLE)
  message(WARNING "'doxygen' not found — API documentation will not be built")
endif()

find_program(SPHINXAUTOBUILD sphinx-autobuild)

# ---------------------------------------------------------------------------
# Helper: add_doc_target — creates both <name> and <name>-nodeps targets
# ---------------------------------------------------------------------------

function(add_doc_target name)
  add_custom_target(${name} ${ARGN})
  add_custom_target(${name}-nodeps ${ARGN})
endfunction()

# ---------------------------------------------------------------------------
# Environment passed to sphinx-build
# ---------------------------------------------------------------------------

set(SPHINX_ENV
  ZEPHYR_BASE=${ZEPHYR_BASE}
  DOCS_HTML_DIR=${DOCS_HTML_DIR}
  OUTPUT_DIR=${DOCS_HTML_DIR}
  DOCS_HTML_BASEURL=https://cfriedt.github.io/posix-next/
  $<$<BOOL:${DOXYGEN_EXECUTABLE}>:DOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE}>
)

set(SPHINX_TAGS_ARGS "-t" "${DOC_TAG}")

# ---------------------------------------------------------------------------
# html target
# ---------------------------------------------------------------------------

add_doc_target(
  html
  COMMAND ${CMAKE_COMMAND} -E env ${SPHINX_ENV} OUTPUT_DIR=${DOCS_HTML_DIR}
    ${SPHINXBUILD}
      -b html
      -c ${DOCS_CFG_DIR}
      -d ${DOCS_DOCTREE_DIR}
      -w ${DOCS_BUILD_DIR}/html.log
      ${SPHINX_TAGS_ARGS}
      ${SPHINXOPTS}
      ${SPHINXOPTS_EXTRA}
      ${DOCS_SRC_DIR}
      ${DOCS_HTML_DIR}
  USES_TERMINAL
  COMMENT "Running Sphinx HTML build..."
)

set_target_properties(html html-nodeps PROPERTIES
  ADDITIONAL_CLEAN_FILES "${DOCS_HTML_DIR};${DOCS_DOCTREE_DIR}"
)

# Interactive SVGs that use <object> embedding need to be copied verbatim
# (Sphinx only copies figure/image SVGs to _images/).
set(INTERACTIVE_SVGS
  posix/overview/aep.svg
)
foreach(svg ${INTERACTIVE_SVGS})
  add_custom_command(TARGET html POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${DOCS_SRC_DIR}/${svg}
      ${DOCS_HTML_DIR}/${svg}
    COMMENT "Copying interactive SVG: ${svg}"
  )
endforeach()

# Rename posix_<name>.h → <name>.h in Doxygen HTML display text so users
# see standard header names instead of the collision-avoidance prefixes.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_custom_command(TARGET html POST_BUILD
  COMMAND ${Python3_EXECUTABLE}
    ${DOCS_SRC_DIR}/scripts/rename_posix_headers.py
    ${DOCS_HTML_DIR}/doxygen/html
  COMMENT "Renaming posix_ header display names in Doxygen output"
)

# ---------------------------------------------------------------------------
# html-live target (auto-rebuild on change)
# ---------------------------------------------------------------------------

if(SPHINXAUTOBUILD)
  add_doc_target(
    html-live
    COMMAND ${CMAKE_COMMAND} -E env ${SPHINX_ENV} OUTPUT_DIR=${DOCS_HTML_DIR}
      ${SPHINXAUTOBUILD}
        --watch ${DOCS_CFG_DIR}
        --ignore ${DOCS_BUILD_DIR}
        -b html
        -c ${DOCS_CFG_DIR}
        -d ${DOCS_DOCTREE_DIR}
        ${SPHINX_TAGS_ARGS}
        ${SPHINXOPTS}
        ${SPHINXOPTS_EXTRA}
        ${DOCS_SRC_DIR}
        ${DOCS_HTML_DIR}
    USES_TERMINAL
    COMMENT "Running Sphinx HTML autobuild..."
  )
endif()

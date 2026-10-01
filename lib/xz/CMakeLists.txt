# SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/xz-embedded)
# the dictionary is allocated per stream; a stream with a check this build
# cannot verify still decodes
zephyr_library_compile_definitions(XZ_DEC_DYNALLOC XZ_DEC_ANY_CHECK)
zephyr_library_sources(
  xz-embedded/xz_crc32.c
  xz-embedded/xz_dec_lzma2.c
  xz-embedded/xz_dec_stream.c
)
zephyr_library_sources_ifdef(CONFIG_XZ_EXTRACT xz_extract.c)

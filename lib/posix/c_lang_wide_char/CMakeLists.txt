# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)

if(NOT CONFIG_TC_PROVIDES_POSIX_C_LANG_WIDE_CHAR AND CONFIG_PICOLIBC)
  zephyr_library_sources(wcstold.c)
endif()

# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)

if(NOT CONFIG_TC_PROVIDES_POSIX_REGEXP)
  zephyr_library_sources(
    regcomp.c
    regerror.c
    regexec.c
    regfree.c
  )
endif()

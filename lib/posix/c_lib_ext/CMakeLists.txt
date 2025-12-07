# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(getopt)

if(CONFIG_TC_PROVIDES_POSIX_C_LIB_EXT)
  return()
endif()

zephyr_library()
zephyr_library_sources(
  fnmatch.c
  getentropy.c
  getopt.c
  getsubopt.c
  stpcpy.c
  strcasecmp.c
  strdup.c
  strfmon.c
  strnlen.c
)

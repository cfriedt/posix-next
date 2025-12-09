# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(getopt)

zephyr_library()
zephyr_library_sources_ifndef(CONFIG_TC_PROVIDES_POSIX_C_LIB_EXT
  fnmatch.c
  getentropy.c
  getopt.c
  getopt_r.c
  getsubopt.c
  stpcpy.c
  strcasecmp.c
  strdup.c
  strfmon.c
  strnlen.c
)

# this allows all users to use getopt_r() even when getopt() is provided externally
zephyr_library_sources_ifdef(CONFIG_POSIX_GETOPT_R getopt_r.c)

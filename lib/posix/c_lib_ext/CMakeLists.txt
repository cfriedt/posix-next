# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(getopt)

zephyr_library()

if(NOT CONFIG_TC_PROVIDES_POSIX_C_LIB_EXT)

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

# technically not part of POSIX_C_LIB_EXT, but closely related
zephyr_library_sources(
  getopt_long.c
  zephyr_getopt.c
)

endif()

# in case CONFIG_TC_PROVIDES_POSIX_C_LIB_EXT is set, and users still want zephyr_getopt()
zephyr_library_sources_ifdef(CONFIG_ZEPHYR_GETOPT zephyr_getopt.c)

zephyr_library_property(ALLOW_EMPTY TRUE)
zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)

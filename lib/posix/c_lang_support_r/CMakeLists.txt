# SPDX-License-Identifier: Apache-2.0

if(NOT CONFIG_TC_PROVIDES_POSIX_C_LANG_SUPPORT_R)

if(TARGET lib__libc__minimal)
  # strerror_r.c unconditionally includes generated libc/minimal/strerror_table.h.
  # That file is always produced by lib__libc__minimal (regardless of
  # CONFIG_MINIMAL_LIBC_STRING_ERROR_TABLE); without this edge, parallel Ninja may
  # compile strerror_r.c before the generator runs.
  add_dependencies(zephyr lib__libc__minimal)
endif()

zephyr_library_sources(
  # zephyr-keep-sorted-start
  asctime_r.c
  ctime_r.c
  qsort_r.c
  rand_r.c
  strerror_r.c
  strtok_r.c
  # zephyr-keep-sorted-stop
)

# zephyr-keep-sorted-start
zephyr_library_sources_ifndef(CONFIG_STD_C23 gmtime_r.c)
zephyr_library_sources_ifndef(CONFIG_STD_C23 localtime_r.c)
# zephyr-keep-sorted-stop

endif()

zephyr_library_property(ALLOW_EMPTY TRUE)
zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)

# SPDX-License-Identifier: Apache-2.0

if(NOT CONFIG_TC_PROVIDES_POSIX_C_LANG_SUPPORT_R)

if(CONFIG_MINIMAL_LIBC_STRING_ERROR_TABLE)
  set(GEN_DIR ${ZEPHYR_BINARY_DIR}/include/generated)
  set(STRERROR_TABLE_H ${GEN_DIR}/libc/minimal/strerror_table.h)

  set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/strerror_r.c
    PROPERTIES OBJECT_DEPENDS ${STRERROR_TABLE_H}
  )
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

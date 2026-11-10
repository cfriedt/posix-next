# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_property(ALLOW_EMPTY TRUE)

zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../shared
)

zephyr_library_compile_options(-U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
zephyr_library_compile_options(-U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(NOT CONFIG_TC_PROVIDES_XSI_FILE_SYSTEM)
  zephyr_library_sources(
    basename.c
    dirname.c
    ftw.c
    lockf.c
    mknod.c
    realpath.c
    seekdir.c
    sync.c
    telldir.c
    tempnam.c
  )
endif()

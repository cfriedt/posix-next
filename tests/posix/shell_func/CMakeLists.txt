# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_shell_func)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
# what an application supplies beside the POSIX surface for the toybox image: the sample's layer
target_sources_ifdef(CONFIG_TOYBOX app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../samples/posix/toybox/src/compat.c
)

target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
target_compile_options(app PRIVATE -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

if(CONFIG_TOYBOX)
  # the toybox module builds toybox.llext; embed it for installation into ext2
  add_dependencies(app toybox_llext)
  generate_inc_file_for_target(app ${TOYBOX_LLEXT_FILE}
    ${ZEPHYR_BINARY_DIR}/include/generated/toybox.inc
  )
endif()

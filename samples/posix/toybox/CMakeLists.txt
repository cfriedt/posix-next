# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_toybox)

target_sources(app PRIVATE src/main.c src/compat.c)

target_compile_options(app PRIVATE
  -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L
  -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700
)

# the toybox module builds toybox.llext; embed it for installation into ext2
add_dependencies(app toybox_llext)
generate_inc_file_for_target(app ${TOYBOX_LLEXT_FILE}
  ${ZEPHYR_BINARY_DIR}/include/generated/toybox.inc
)

# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_exec)

FILE(GLOB app_sources src/main.c)
target_sources(app PRIVATE ${app_sources})

target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)

# the executable image under test: a real ELF extension
add_llext_target(hello_ext
  OUTPUT  ${PROJECT_BINARY_DIR}/llext/hello.llext
  SOURCES ${PROJECT_SOURCE_DIR}/src/hello_ext.c
)
generate_inc_file_for_target(app ${PROJECT_BINARY_DIR}/llext/hello.llext
  ${ZEPHYR_BINARY_DIR}/include/generated/hello_ext.inc
)

# a loadable object that exports no main(): execve() must refuse it
add_llext_target(noexec_ext
  OUTPUT  ${PROJECT_BINARY_DIR}/llext/noexec.llext
  SOURCES ${PROJECT_SOURCE_DIR}/src/noexec_ext.c
)
generate_inc_file_for_target(app ${PROJECT_BINARY_DIR}/llext/noexec.llext
  ${ZEPHYR_BINARY_DIR}/include/generated/noexec_ext.inc
)

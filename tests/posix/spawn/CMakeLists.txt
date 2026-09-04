# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_spawn)

target_sources(app PRIVATE src/main.c src/image_registry.c)

target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/posix/spawn
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/posix/shared
)
zephyr_linker_sources(ROM_SECTIONS iterables.ld)

if(CONFIG_POSIX_EXEC_LLEXT)
  # file system images: a real ELF extension, and one that exports no main()
  add_llext_target(hello_ext
    OUTPUT  ${PROJECT_BINARY_DIR}/llext/hello.llext
    SOURCES ${PROJECT_SOURCE_DIR}/src/hello_ext.c
  )
  generate_inc_file_for_target(app ${PROJECT_BINARY_DIR}/llext/hello.llext
    ${ZEPHYR_BINARY_DIR}/include/generated/hello_ext.inc
  )
  add_llext_target(noexec_ext
    OUTPUT  ${PROJECT_BINARY_DIR}/llext/noexec.llext
    SOURCES ${PROJECT_SOURCE_DIR}/src/noexec_ext.c
  )
  generate_inc_file_for_target(app ${PROJECT_BINARY_DIR}/llext/noexec.llext
    ${ZEPHYR_BINARY_DIR}/include/generated/noexec_ext.inc
  )
endif()

# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_threads_base)

file(GLOB app_sources src/*.c)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/posix)

target_sources(app PRIVATE
  ${app_sources}
  ../common/msleep.c
  ../common/test_sigmask_common.c
)

target_include_directories(app PRIVATE
  ${ZEPHYR_POSIX_NEXT_MODULE_DIR}/lib/posix/options/timers
  ${ZEPHYR_POSIX_NEXT_MODULE_DIR}/lib/posix/options/shared
)

target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
target_compile_options(app PRIVATE -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

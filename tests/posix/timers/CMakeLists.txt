# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_timers)

FILE(GLOB app_sources src/*.c)

target_sources(app PRIVATE ${app_sources})

# for private posix_clock.h, used for conversion functions
target_include_directories(app PRIVATE ${ZEPHYR_POSIX_NEXT_MODULE_DIR}/lib/posix/options)

# FIXME: We should not rely on usleep() (which is was dropped in Issue 7, 200809L)
target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200112L)
target_compile_options(app PRIVATE -U_XOPEN_SOURCE -D_XOPEN_SOURCE=600)

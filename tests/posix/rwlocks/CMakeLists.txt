# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_rw_locks)

target_sources(app PRIVATE src/main.c)

# FIXME: We should not rely on usleep() (which is was dropped in Issue 7, 200809L)
target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200112L)
target_compile_options(app PRIVATE -U_XOPEN_SOURCE -D_XOPEN_SOURCE=600)

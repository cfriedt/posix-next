# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_sporadic_server)

target_sources(app PRIVATE
  src/main.c
  src/sched_get_priority_max.c
  src/sched_get_priority_min.c
  src/sched_getparam.c
  src/sched_getscheduler.c
  src/sched_setparam.c
  src/sched_setscheduler.c
)

target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)

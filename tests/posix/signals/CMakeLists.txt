# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(posix_signals)

target_sources(app PRIVATE
  src/main.c
  src/test_default_action.c
  src/test_generate.c
  src/test_sigaction.c
  src/test_wait.c
  ../shared/test_sigmask_common.c
)

target_compile_options(app PRIVATE -U_POSIX_C_SOURCE -D_POSIX_C_SOURCE=200809L)
# the ucontext_t a three-argument handler receives is XSI
target_compile_options(app PRIVATE -U_XOPEN_SOURCE -D_XOPEN_SOURCE=700)

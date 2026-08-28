/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits.h>

#include <zephyr/ztest.h>

BUILD_ASSERT(_POSIX_PIPE_BUF == 512, "_POSIX_PIPE_BUF is not defined to 512");
BUILD_ASSERT(PIPE_BUF >= _POSIX_PIPE_BUF, "PIPE_BUF is below the POSIX minimum");

ZTEST_SUITE(posix_pipe, NULL, NULL, NULL, NULL, NULL);

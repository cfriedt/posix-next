/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_FIFO_TESTS_H_
#define POSIX_TESTS_FIFO_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

#define FS_TMPDIR    "/tmp"
#define TEST_ROOT    FS_TMPDIR "/pfifo"
#define TEST_FIFO    TEST_ROOT "/fifo"
#define TEST_FILE    TEST_ROOT "/file.txt"
#define TEST_NOENT   TEST_ROOT "/nope"
#define TEST_MESSAGE "through the fifo"

#endif /* POSIX_TESTS_FIFO_TESTS_H_ */

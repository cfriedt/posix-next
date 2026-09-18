/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_SYMBOLIC_LINKS_TESTS_H_
#define POSIX_TESTS_SYMBOLIC_LINKS_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

#define FS_TMPDIR    "/tmp"
#define TEST_ROOT    FS_TMPDIR "/pslnk"
#define TEST_FILE    TEST_ROOT "/file.txt"
#define TEST_LINK    TEST_ROOT "/link"
#define TEST_DANGLE  TEST_ROOT "/dangle"
#define TEST_NOENT   TEST_ROOT "/nope"
#define TEST_CONTENT "The quick brown fox jumps over the lazy dog!"

/* fixture descriptor, opened and granted per test */
extern int fs_test_fd; /* TEST_FILE, O_RDWR */

#endif /* POSIX_TESTS_SYMBOLIC_LINKS_TESTS_H_ */

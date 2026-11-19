/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_FILE_SYSTEM_FD_COMMON_H_
#define POSIX_TESTS_FILE_SYSTEM_FD_COMMON_H_

#include "file_system_fd_tests.h"

#include <fcntl.h>
#include <unistd.h>

/* a descriptor for TEST_ROOT, the directory the relative names resolve against */
static inline int open_test_root(void)
{
	int fd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);

	zassert_true(fd >= 0, "open(" TEST_ROOT ") failed: %d", errno);

	return fd;
}

#endif /* POSIX_TESTS_FILE_SYSTEM_FD_COMMON_H_ */

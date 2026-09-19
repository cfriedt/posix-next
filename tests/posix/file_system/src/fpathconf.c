/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_fpathconf)
{
	zassert_true(fpathconf(fs_test_fd, _PC_PATH_MAX) > 0);
	IF_NOT_NATIVE_LIBC({
		zassert_equal(fpathconf(fs_test_fd, _PC_LINK_MAX), 1);
	});

	/* special characters belong to terminals: the console has them, a file has no
	 * association (the host reports the value for any file)
	 */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(fpathconf(fs_test_fd, _PC_VDISABLE), -1);
		zassert_equal(errno, 0);
		zassert_equal(fpathconf(STDIN_FILENO, _PC_VDISABLE), _POSIX_VDISABLE);
	});
}

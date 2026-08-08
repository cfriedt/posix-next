/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_system, test_pathconf)
{
	zassert_true(pathconf(TEST_FILE, _PC_PATH_MAX) > 0);
	zassert_true(pathconf(TEST_FILE, _PC_NAME_MAX) > 0);
	/* LINK_MAX == 1 is a Zephyr deviation; the host reports its own value */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(pathconf(TEST_FILE, _PC_LINK_MAX), 1);
	});

	/* MAX_CANON has no association here: -1 with errno unchanged (Zephyr) */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(pathconf(TEST_FILE, _PC_MAX_CANON), -1);
		zassert_equal(errno, 0);
	});
}

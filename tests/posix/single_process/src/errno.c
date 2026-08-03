/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_single_process, test_errno)
{
	errno = 0;
	zassert_equal(errno, 0);
	errno = ERANGE;
	zassert_equal(errno, ERANGE);
	errno = 0;
}

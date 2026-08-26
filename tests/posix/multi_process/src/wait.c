/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/wait.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_wait)
{
	int status;

	errno = 0;
	zexpect_equal(wait(&status), -1);
	zexpect_equal(errno, ECHILD);
}

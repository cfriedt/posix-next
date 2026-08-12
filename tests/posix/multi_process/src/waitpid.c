/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/wait.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_waitpid)
{
	int status;

	errno = 0;
	zexpect_equal(waitpid(-1, &status, 0), -1);
	zexpect_equal(errno, ECHILD);

	errno = 0;
	zexpect_equal(waitpid(-1, &status, WNOHANG), -1);
	zexpect_equal(errno, ECHILD);

	/* WNOWAIT is a waitid()-only option; waitpid() rejects it */
	errno = 0;
	zexpect_equal(waitpid(-1, &status, WNOWAIT), -1);
	zexpect_equal(errno, EINVAL);
}

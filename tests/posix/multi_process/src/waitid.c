/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/wait.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_waitid)
{
	siginfo_t info;

	errno = 0;
	zexpect_equal(waitid(P_ALL, 0, &info, WEXITED), -1);
	zexpect_equal(errno, ECHILD);

	errno = 0;
	zexpect_equal(waitid(P_PID, 1, &info, WEXITED | WNOHANG | WNOWAIT), -1);
	zexpect_equal(errno, ECHILD);

	errno = 0;
	zexpect_equal(waitid((idtype_t)0xdead, 0, &info, WEXITED), -1);
	zexpect_equal(errno, EINVAL);
}

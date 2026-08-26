/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_setpgid)
{
	/* a negative process or group id is invalid */
	errno = 0;
	zexpect_equal(setpgid(-1, 0), -1);
	zexpect_equal(errno, EINVAL);

	errno = 0;
	zexpect_equal(setpgid(0, -1), -1);
	zexpect_equal(errno, EINVAL);

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		pid_t self = getpid();

		/* a nonexistent pid is ESRCH; on the host self + 1 may be a live process */
		errno = 0;
		zexpect_equal(setpgid(self + 1, 0), -1);
		zexpect_equal(errno, ESRCH);
	}
}

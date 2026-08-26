/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_getpgid)
{
	pid_t self = getpid();

	zexpect_equal(getpgid(0), getpgrp());
	zexpect_equal(getpgid(self), getpgrp());

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* a nonexistent pid is ESRCH; on the host self + 1 may be a live process */
		errno = 0;
		zexpect_equal(getpgid(self + 1), -1);
		zexpect_equal(errno, ESRCH);
	}
}

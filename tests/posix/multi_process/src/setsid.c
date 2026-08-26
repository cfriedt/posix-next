/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_setsid)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* host setsid() succeeds for a non-group-leader; skip the EPERM check */
		ztest_test_skip();
		return;
	}

	errno = 0;
	zexpect_equal(setsid(), -1);
	zexpect_equal(errno, EPERM);
}

/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_fork)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC) || IS_ENABLED(CONFIG_PROCESS_VM)) {
		/* the fork round-trip is exercised by tests/posix/fork */
		ztest_test_skip();
		return;
	}

	errno = 0;
	zexpect_equal(fork(), -1);
	zexpect_equal(errno, ENOTSUP);
}

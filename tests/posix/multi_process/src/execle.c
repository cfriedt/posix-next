/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_execle)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* host libc actually execs/forks; the prelinked-image resolution is Zephyr-only */
		ztest_test_skip();
		return;
	}


	errno = 0;
	zexpect_equal(execle("/bin/true", "true", NULL, NULL), -1);
	zexpect_equal(errno, ENOENT);
}

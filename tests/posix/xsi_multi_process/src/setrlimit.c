/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/resource.h>

#include <zephyr/ztest.h>

ZTEST_USER(xsi_multi_process, test_setrlimit)
{
	struct rlimit rl;
	struct rlimit bad;

	zassert_ok(getrlimit(RLIMIT_NOFILE, &rl));

	/* the current limits are accepted as they are */
	zassert_ok(setrlimit(RLIMIT_NOFILE, &rl));

	/* a soft limit above the hard limit is no limit */
	if (rl.rlim_max != RLIM_INFINITY) {
		bad = rl;
		bad.rlim_cur = bad.rlim_max + 1;
		errno = 0;
		zassert_equal(setrlimit(RLIMIT_NOFILE, &bad), -1);
		zassert_equal(errno, EINVAL);
	}

	errno = 0;
	zassert_equal(setrlimit(-1, &rl), -1);
	zassert_equal(errno, EINVAL);

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the limits are the build's: a change, up or down, is refused */
		bad = rl;
		bad.rlim_cur = rl.rlim_cur - 1;
		errno = 0;
		zassert_equal(setrlimit(RLIMIT_NOFILE, &bad), -1);
		zassert_equal(errno, EPERM);
		bad = rl;
		bad.rlim_max = rl.rlim_max + 1;
		bad.rlim_cur = bad.rlim_max;
		errno = 0;
		zassert_equal(setrlimit(RLIMIT_NOFILE, &bad), -1);
		zassert_equal(errno, EPERM);
		/* and what it refused is still in force */
		zassert_ok(getrlimit(RLIMIT_NOFILE, &bad));
		zassert_equal(bad.rlim_cur, rl.rlim_cur);
		zassert_equal(bad.rlim_max, rl.rlim_max);
	}
}

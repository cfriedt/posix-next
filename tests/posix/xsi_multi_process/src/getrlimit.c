/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/resource.h>

#include <zephyr/ztest.h>

ZTEST_USER(xsi_multi_process, test_getrlimit)
{
	static const int resources[] = {
		RLIMIT_CORE, RLIMIT_CPU, RLIMIT_DATA, RLIMIT_FSIZE, RLIMIT_NOFILE, RLIMIT_STACK, RLIMIT_AS,
	};
	struct rlimit rl;

	/* every resource POSIX names has a pair of limits, soft within hard */
	ARRAY_FOR_EACH(resources, i) {
		zassert_ok(getrlimit(resources[i], &rl), "resource %d: %d", resources[i], errno);
		zassert_true(rl.rlim_cur <= rl.rlim_max, "resource %d: soft limit above the hard one",
			     resources[i]);
	}

	zassert_ok(getrlimit(RLIMIT_NOFILE, &rl));
	zassert_true(rl.rlim_cur > 0);
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the build's limits, hard equal to soft */
		zassert_equal(rl.rlim_cur, ZVFS_OPEN_SIZE);
		zassert_equal(rl.rlim_max, ZVFS_OPEN_SIZE);
		zassert_ok(getrlimit(RLIMIT_CORE, &rl));
		zassert_equal(rl.rlim_cur, 0, "a core file is never written");
		zassert_ok(getrlimit(RLIMIT_CPU, &rl));
		zassert_equal(rl.rlim_cur, RLIM_INFINITY);
	}

	errno = 0;
	zassert_equal(getrlimit(-1, &rl), -1);
	zassert_equal(errno, EINVAL);
	errno = 0;
	zassert_equal(getrlimit(1000, &rl), -1);
	zassert_equal(errno, EINVAL);
}

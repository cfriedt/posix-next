/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/resource.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

static uint64_t cpu_us(const struct rusage *ru)
{
	return ((uint64_t)ru->ru_utime.tv_sec * USEC_PER_SEC + ru->ru_utime.tv_usec) +
	       ((uint64_t)ru->ru_stime.tv_sec * USEC_PER_SEC + ru->ru_stime.tv_usec);
}

ZTEST_USER(xsi_multi_process, test_getrusage)
{
	struct rusage before;
	struct rusage after;

	errno = 0;
	zassert_equal(getrusage(42, &after), -1);
	zassert_equal(errno, EINVAL);

	/* the calling process's CPU time moves while it computes */
	zassert_ok(getrusage(RUSAGE_SELF, &before));
	k_busy_wait(50 * USEC_PER_MSEC);
	zassert_ok(getrusage(RUSAGE_SELF, &after));
	zassert_true(cpu_us(&after) > cpu_us(&before), "CPU time did not advance: %llu -> %llu",
		     cpu_us(&before), cpu_us(&after));
	zassert_true(after.ru_utime.tv_usec >= 0 && after.ru_utime.tv_usec < USEC_PER_SEC);

	/* nothing has been waited for: the children's time is nil */
	zassert_ok(getrusage(RUSAGE_CHILDREN, &after));
	zassert_equal(cpu_us(&after), 0);
	zassert_equal(after.ru_maxrss, 0);
}

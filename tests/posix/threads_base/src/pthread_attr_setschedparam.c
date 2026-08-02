/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_attr_setschedparam(void)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	struct sched_param param = {0};

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setschedparam(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_setschedparam(NULL, &param), EINVAL);
			zassert_equal(pthread_attr_setschedparam((pthread_attr_t *)&uninit_attr,
								 &param),
				      EINVAL);
		}
		/* avoid glibc non-null compiler warning promoted to error */
		zassert_equal(pthread_attr_setschedparam(&test_attr, NULL), EINVAL);

		zassert_equal(pthread_attr_setschedparam(
				      &test_attr, &(struct sched_param){.sched_priority = INT_MAX}),
			      EINVAL);
	})

	zassert_ok(pthread_attr_setschedparam(&test_attr, &param));

	can_create_thread(&test_attr);
#else
	ztest_test_skip();
#endif
}

ZTEST_THREADS_BASE(test_pthread_attr_setschedparam);

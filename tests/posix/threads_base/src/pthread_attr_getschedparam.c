/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

#define BIOS_FOOD 0xB105F00D

static void test_pthread_attr_getschedparam(void)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	struct sched_param param = {
		.sched_priority = BIOS_FOOD,
	};

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getschedparam(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getschedparam(NULL, &param), EINVAL);
			zassert_equal(pthread_attr_getschedparam(&uninit_attr, &param), EINVAL);
		}
		zassert_equal(pthread_attr_getschedparam(&test_attr, NULL), EINVAL);
	})

	/* only check to see that the function succeeds and sets param */
	zassert_ok(pthread_attr_getschedparam(&test_attr, &param));
	zassert_not_equal(BIOS_FOOD, param.sched_priority);
#else
	ztest_test_skip();
#endif
}

ZTEST_THREADS_BASE(test_pthread_attr_getschedparam);

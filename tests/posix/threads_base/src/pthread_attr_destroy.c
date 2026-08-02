/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_attr_destroy(void)
{
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_destroy((pthread_attr_t *)&uninit_attr), EINVAL);
		}
	})

	/* can destroy an initialized attr */
	zassert_ok(pthread_attr_destroy(&test_attr), "failed to destroy an initialized attr");
	test_attr_valid = false;

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* glibc does not return EINVAL passing an invalid attribute to pthread_create()
		 * (POSIX non-conformance)
		 */
		cannot_create_thread(&test_attr);
	}

	if (false) {
		/* undefined behaviour */
		zassert_ok(pthread_attr_destroy(&test_attr));
	}

	/* re-initialize so that after() has a valid attr to clean up */
	zassert_ok(pthread_attr_init(&test_attr));
	test_attr_valid = true;
}

ZTEST_THREADS_BASE(test_pthread_attr_destroy);

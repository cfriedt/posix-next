/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void test_pthread_condattr_setpshared(void)
{
#ifdef _POSIX_THREAD_PROCESS_SHARED
	int pshared = -1;
	pthread_condattr_t att = {0};
	pthread_cond_t cv;

	zassert_ok(pthread_condattr_init(&att));

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		pthread_condattr_t uninit = {0};

		zassert_equal(pthread_condattr_setpshared(NULL, PTHREAD_PROCESS_SHARED), EINVAL);
		zassert_equal(pthread_condattr_setpshared(&uninit, PTHREAD_PROCESS_SHARED),
			      EINVAL);
	})
	zassert_equal(pthread_condattr_setpshared(&att, 42), EINVAL);
	zassert_ok(pthread_condattr_getpshared(&att, &pshared));
	zassert_equal(pshared, PTHREAD_PROCESS_PRIVATE, "a rejected value changed the attribute");

	zassert_ok(pthread_condattr_setpshared(&att, PTHREAD_PROCESS_SHARED));
	zassert_ok(pthread_condattr_getpshared(&att, &pshared));
	zassert_equal(pshared, PTHREAD_PROCESS_SHARED);
	zassert_ok(pthread_condattr_setpshared(&att, PTHREAD_PROCESS_PRIVATE));
	zassert_ok(pthread_condattr_getpshared(&att, &pshared));
	zassert_equal(pshared, PTHREAD_PROCESS_PRIVATE);

	/* a shared condition variable initializes and signals like a private one */
	zassert_ok(pthread_condattr_setpshared(&att, PTHREAD_PROCESS_SHARED));
	zassert_ok(pthread_cond_init(&cv, &att));
	zassert_ok(pthread_cond_signal(&cv));
	zassert_ok(pthread_cond_destroy(&cv));

	zassert_ok(pthread_condattr_destroy(&att));
#else
	ztest_test_skip();
#endif
}

ZTEST_THREADS_BASE(test_pthread_condattr_setpshared);

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

#if !(_POSIX_C_SOURCE >= 202405L)
/* pthread_cond_clockwait() was standardized in POSIX Issue 8 */
int pthread_cond_clockwait(pthread_cond_t *cond, pthread_mutex_t *mutex, clockid_t clock_id,
			   const struct timespec *abstime);
#endif

ZTEST_USER(posix_clock_selection, test_pthread_cond_clockwait)
{
	int ret;
	pthread_cond_t cond;
	pthread_mutex_t mtx;
	pthread_condattr_t att = {0};
	const struct timespec past = {0};

	zassert_ok(pthread_condattr_init(&att));
	zassert_ok(pthread_condattr_setclock(&att, CLOCK_MONOTONIC));
	zassert_ok(pthread_cond_init(&cond, &att));
	zassert_ok(pthread_condattr_destroy(&att));
	zassert_ok(pthread_mutex_init(&mtx, NULL));

	zassert_ok(pthread_mutex_lock(&mtx));

	/* an already-past absolute deadline times out without blocking */
	ret = pthread_cond_clockwait(&cond, &mtx, CLOCK_MONOTONIC, &past);
	zassert_equal(ret, ETIMEDOUT, "expected ETIMEDOUT, got %d", ret);

	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_cond_clockwait(&cond, &mtx, CLOCK_MONOTONIC, NULL), EINVAL);
	})

	zassert_ok(pthread_mutex_unlock(&mtx));
	zassert_ok(pthread_mutex_destroy(&mtx));
	zassert_ok(pthread_cond_destroy(&cond));
}

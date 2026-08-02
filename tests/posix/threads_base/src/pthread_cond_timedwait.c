/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

#define TIMEDWAIT_TIMEOUT_MS       200
#define TIMEDWAIT_TIMEOUT_DELAY_MS 100

BUILD_ASSERT(TIMEDWAIT_TIMEOUT_DELAY_MS >= 100, "TIMEDWAIT_TIMEOUT_DELAY_MS too small");
BUILD_ASSERT(TIMEDWAIT_TIMEOUT_MS >= 2 * TIMEDWAIT_TIMEOUT_DELAY_MS,
	     "TIMEDWAIT_TIMEOUT_MS too small");

static ZTEST_BMEM pthread_mutex_t cond_timedwait_mtx;
static ZTEST_BMEM pthread_cond_t cond_timedwait_cv;

static inline void timespec_add_ms(struct timespec *ts, uint32_t ms)
{
	struct timespec addend;

	timespec_from_timeout(K_MSEC(ms), &addend);
	timespec_add(ts, &addend);
}

static void cond_timedwait_static_init(void)
{
	int rc;
	pthread_cond_t static_cv = PTHREAD_COND_INITIALIZER;
	const struct timespec past = {0};

	zassert_ok(pthread_mutex_lock(&cond_timedwait_mtx));
	rc = pthread_cond_timedwait(&static_cv, &cond_timedwait_mtx, &past);
	zassert_equal(rc, ETIMEDOUT, "expected ETIMEDOUT, got %d", rc);
	zassert_ok(pthread_mutex_unlock(&cond_timedwait_mtx));
	zassert_ok(pthread_cond_destroy(&static_cv));
}

static void *cond_timedwait_fn(void *arg)
{
	int ret;
	struct timespec time_point;
	pthread_mutex_t *mtx = (pthread_mutex_t *)arg;

	zassume_ok(clock_gettime(CLOCK_REALTIME, &time_point));
	timespec_add_ms(&time_point, TIMEDWAIT_TIMEOUT_MS);

	zassert_ok(pthread_mutex_lock(mtx));
	ret = pthread_cond_timedwait(&cond_timedwait_cv, mtx, &time_point);
	zassert_ok(pthread_mutex_unlock(mtx));

	return INT_TO_POINTER(ret);
}

static void test_pthread_cond_timedwait(void)
{
	void *ret;
	pthread_t th;

	posix_test_skip_if_native_libc();

	zassert_ok(pthread_mutex_init(&cond_timedwait_mtx, NULL));
	zassert_ok(pthread_cond_init(&cond_timedwait_cv, NULL));

	printk("Expecting timedwait with timeout of %d ms to fail\n", TIMEDWAIT_TIMEOUT_MS);
	zassert_ok(pthread_create(&th, NULL, cond_timedwait_fn, &cond_timedwait_mtx));
	zassert_ok(pthread_join(th, &ret));
	zassert_equal(ETIMEDOUT, POINTER_TO_INT(ret));

	printk("Expecting timedwait with timeout of %d ms to succeed after %d ms\n",
	       TIMEDWAIT_TIMEOUT_MS, TIMEDWAIT_TIMEOUT_DELAY_MS);
	zassert_ok(pthread_create(&th, NULL, cond_timedwait_fn, &cond_timedwait_mtx));
	k_msleep(TIMEDWAIT_TIMEOUT_DELAY_MS);
	zassert_ok(pthread_mutex_lock(&cond_timedwait_mtx));
	zassert_ok(pthread_cond_signal(&cond_timedwait_cv));
	zassert_ok(pthread_mutex_unlock(&cond_timedwait_mtx));
	zassert_ok(pthread_join(th, &ret));
	zassert_equal(0, POINTER_TO_INT(ret));

	IF_NOT_NATIVE_LIBC({
		zassert_ok(pthread_mutex_lock(&cond_timedwait_mtx));
		zassert_equal(EINVAL,
			      pthread_cond_timedwait(&cond_timedwait_cv, &cond_timedwait_mtx,
						     NULL));
		zassert_ok(pthread_mutex_unlock(&cond_timedwait_mtx));
	})

	/* an already-past absolute deadline times out without blocking, mutex re-acquired */
	IF_NOT_NATIVE_LIBC({
		int rc;
		int64_t start;
		const struct timespec past = {0};

		zassert_ok(pthread_mutex_lock(&cond_timedwait_mtx));
		start = k_uptime_get();
		rc = pthread_cond_timedwait(&cond_timedwait_cv, &cond_timedwait_mtx, &past);
		zassert_equal(rc, ETIMEDOUT, "expected ETIMEDOUT, got %d", rc);
		zassert_true(k_uptime_get() - start < TIMEDWAIT_TIMEOUT_DELAY_MS,
			     "past deadline blocked");
		zassert_ok(pthread_mutex_unlock(&cond_timedwait_mtx));
	})

	cond_timedwait_static_init();

	zassert_ok(pthread_cond_destroy(&cond_timedwait_cv));
	zassert_ok(pthread_mutex_destroy(&cond_timedwait_mtx));
}

ZTEST_THREADS_BASE(test_pthread_cond_timedwait);

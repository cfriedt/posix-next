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

#define TIMEDLOCK_TIMEOUT_MS       200
#define TIMEDLOCK_TIMEOUT_DELAY_MS 100

BUILD_ASSERT(TIMEDLOCK_TIMEOUT_DELAY_MS >= 100, "TIMEDLOCK_TIMEOUT_DELAY_MS too small");
BUILD_ASSERT(TIMEDLOCK_TIMEOUT_MS >= 2 * TIMEDLOCK_TIMEOUT_DELAY_MS,
	     "TIMEDLOCK_TIMEOUT_MS too small");

static ZTEST_BMEM pthread_mutex_t timedlock_mutex;

static inline void timespec_add_ms(struct timespec *ts, uint32_t ms)
{
	struct timespec addend;

	timespec_from_timeout(K_MSEC(ms), &addend);
	timespec_add(ts, &addend);
}

static void *mutex_timedlock_fn(void *arg)
{
	int ret;
	struct timespec time_point;
	pthread_mutex_t *mtx = (pthread_mutex_t *)arg;

	zassume_ok(clock_gettime(CLOCK_REALTIME, &time_point));
	timespec_add_ms(&time_point, TIMEDLOCK_TIMEOUT_MS);

	ret = pthread_mutex_timedlock(mtx, &time_point);
	if (ret != 0) {
		return INT_TO_POINTER(ret);
	}

	zassert_ok(pthread_mutex_unlock(mtx));

	return NULL;
}

static void *mutex_timedlock_past_fn(void *arg)
{
	int ret;
	int64_t start;
	struct timespec past = {0};
	pthread_mutex_t *mtx = (pthread_mutex_t *)arg;

	/* an already-past absolute deadline times out without blocking */
	start = k_uptime_get();
	ret = pthread_mutex_timedlock(mtx, &past);
	zassert_true(k_uptime_get() - start < TIMEDLOCK_TIMEOUT_DELAY_MS,
		     "past deadline blocked");

	return INT_TO_POINTER(ret);
}

static void test_pthread_mutex_timedlock(void)
{
	void *ret;
	pthread_t th;

	posix_test_skip_if_native_libc();

	zassert_ok(pthread_mutex_init(&timedlock_mutex, NULL));

	printk("Expecting timedlock with timeout of %d ms to fail\n", TIMEDLOCK_TIMEOUT_MS);
	zassert_ok(pthread_mutex_lock(&timedlock_mutex));
	zassert_ok(pthread_create(&th, NULL, mutex_timedlock_fn, &timedlock_mutex));
	zassert_ok(pthread_join(th, &ret));
	/* ensure timeout occurs */
	zassert_equal(ETIMEDOUT, POINTER_TO_INT(ret));

	printk("Expecting timedlock with timeout of %d ms to succeed after 100ms\n",
	       TIMEDLOCK_TIMEOUT_MS);
	zassert_ok(pthread_create(&th, NULL, mutex_timedlock_fn, &timedlock_mutex));
	/* unlock before timeout expires */
	k_msleep(TIMEDLOCK_TIMEOUT_DELAY_MS);
	zassert_ok(pthread_mutex_unlock(&timedlock_mutex));
	zassert_ok(pthread_join(th, &ret));
	/* ensure lock is successful, in spite of delay  */
	zassert_ok(POINTER_TO_INT(ret));

	/* an already-past absolute deadline times out without blocking */
	zassert_ok(pthread_mutex_lock(&timedlock_mutex));
	zassert_ok(pthread_create(&th, NULL, mutex_timedlock_past_fn, &timedlock_mutex));
	zassert_ok(pthread_join(th, &ret));
	zassert_equal(POINTER_TO_INT(ret), ETIMEDOUT, "expected ETIMEDOUT, got %d",
		      (int)POINTER_TO_INT(ret));
	zassert_ok(pthread_mutex_unlock(&timedlock_mutex));

	zassert_ok(pthread_mutex_destroy(&timedlock_mutex));
}

ZTEST_THREADS_BASE(test_pthread_mutex_timedlock);

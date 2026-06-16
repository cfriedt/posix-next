/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <zephyr/sys/util.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/ztest.h>

#define SLEEP_MS 100

static pthread_mutex_t mutex;

static void *normal_mutex_entry(void *p1)
{
	int i, rc;

	/* Sleep for maximum 300 ms as main thread is sleeping for 100 ms */

	for (i = 0; i < 3; i++) {
		rc = pthread_mutex_trylock(&mutex);
		if (rc == 0) {
			break;
		}
		k_msleep(SLEEP_MS);
	}

	zassert_false(rc, "try lock failed");
	TC_PRINT("mutex lock is taken\n");
	zassert_false(pthread_mutex_unlock(&mutex), "mutex unlock is failed");
	return NULL;
}

/**
 * @brief Test default mutex trylock and lock behaviour
 *
 * @details Uses the default mutex attributes. pthread_mutex_trylock
 *	    and pthread_mutex_lock are exercised without
 *	    pthread_mutexattr_settype().
 */
ZTEST(mutex, test_mutex_normal)
{
	__maybe_unused pthread_t th;

	zassert_ok(pthread_mutex_init(&mutex, NULL));

	zassert_ok(pthread_mutex_lock(&mutex));

	if (CONFIG_SYS_THREAD_STACK_MAX > 0) {
		zassert_ok(pthread_create(&th, NULL, normal_mutex_entry, NULL));
	}

	k_msleep(SLEEP_MS);
	zassert_ok(pthread_mutex_unlock(&mutex));

	if (CONFIG_SYS_THREAD_STACK_MAX > 0) {
		zassert_ok(pthread_join(th, NULL));
	}

	zassert_ok(pthread_mutex_destroy(&mutex));
}

/**
 * @brief Test to demonstrate limited mutex resources
 *
 * @details Exactly SYS_THREAD_MUTEX_MIN can be in use at once (when heap allocation is
 * unavailable).
 */
ZTEST(mutex, test_mutex_resource_exhausted)
{
	size_t i;
	pthread_mutex_t m[SYS_THREAD_MUTEX_MIN + 1];

	for (i = 0; i < SYS_THREAD_MUTEX_MIN; ++i) {
		zassert_ok(pthread_mutex_init(&m[i], NULL), "failed to init mutex %zu", i);
	}

	/* try to initialize one more than SYS_THREAD_MUTEX_MIN */
	zassert_equal(i, SYS_THREAD_MUTEX_MIN);

	if (SYS_THREAD_MUTEX_MIN == CONFIG_SYS_THREAD_MUTEX_MAX) {
		/* This test may be removed eventally, since this assertion is successful only when
		 * heap allocation is unavailable, which is non-standard.
		 */
		zassert_not_equal(0, pthread_mutex_init(&m[i], NULL),
				  "should not have initialized mutex %zu", i);
	}

	for (; i > 0; --i) {
		zassert_ok(pthread_mutex_destroy(&m[i - 1]), "failed to destroy mutex %zu", i - 1);
	}
}

/**
 * @brief Test to that there are no mutex resource leaks
 *
 * @details Demonstrate that mutexes may be used over and over again.
 */
ZTEST(mutex, test_mutex_resource_leak)
{
	pthread_mutex_t m;

	for (size_t i = 0; i < 2 * SYS_THREAD_MUTEX_MIN; ++i) {
		zassert_ok(pthread_mutex_init(&m, NULL), "failed to init mutex %zu", i);
		zassert_ok(pthread_mutex_destroy(&m), "failed to destroy mutex %zu", i);
	}
}

#define TIMEDLOCK_TIMEOUT_MS       200
#define TIMEDLOCK_TIMEOUT_DELAY_MS 100

BUILD_ASSERT(TIMEDLOCK_TIMEOUT_DELAY_MS >= 100, "TIMEDLOCK_TIMEOUT_DELAY_MS too small");
BUILD_ASSERT(TIMEDLOCK_TIMEOUT_MS >= 2 * TIMEDLOCK_TIMEOUT_DELAY_MS,
	     "TIMEDLOCK_TIMEOUT_MS too small");

static inline void timespec_add_ms(struct timespec *ts, uint32_t ms)
{
	struct timespec addend;

	timespec_from_timeout(K_MSEC(ms), &addend);
	timespec_add(ts, &addend);
}

static void *test_mutex_timedlock_fn(void *arg)
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

/** @brief Test to verify @ref pthread_mutex_timedlock returns ETIMEDOUT */
ZTEST(mutex, test_mutex_timedlock)
{
	void *ret;
	pthread_t th;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * threads_base.static_stack uses statically allocated stacks.
		 */
		ztest_test_skip();
	}

	zassert_ok(pthread_mutex_init(&mutex, NULL));

	printk("Expecting timedlock with timeout of %d ms to fail\n", TIMEDLOCK_TIMEOUT_MS);
	zassert_ok(pthread_mutex_lock(&mutex));
	zassert_ok(pthread_create(&th, NULL, test_mutex_timedlock_fn, &mutex));
	zassert_ok(pthread_join(th, &ret));
	/* ensure timeout occurs */
	zassert_equal(ETIMEDOUT, POINTER_TO_INT(ret));

	printk("Expecting timedlock with timeout of %d ms to succeed after 100ms\n",
	       TIMEDLOCK_TIMEOUT_MS);
	zassert_ok(pthread_create(&th, NULL, test_mutex_timedlock_fn, &mutex));
	/* unlock before timeout expires */
	k_msleep(TIMEDLOCK_TIMEOUT_DELAY_MS);
	zassert_ok(pthread_mutex_unlock(&mutex));
	zassert_ok(pthread_join(th, &ret));
	/* ensure lock is successful, in spite of delay  */
	zassert_ok(POINTER_TO_INT(ret));

	zassert_ok(pthread_mutex_destroy(&mutex));
}

static void before(void *data)
{
	ARG_UNUSED(data);

	mutex = PTHREAD_MUTEX_INITIALIZER;
}

ZTEST_SUITE(mutex, NULL, NULL, before, NULL, NULL);

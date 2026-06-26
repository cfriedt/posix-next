/*
 * Copyright (c) 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"
#include "_main.h"

#define TIMEDWAIT_TIMEOUT_MS       200
#define TIMEDWAIT_TIMEOUT_DELAY_MS 100

BUILD_ASSERT(TIMEDWAIT_TIMEOUT_DELAY_MS >= 100, "TIMEDWAIT_TIMEOUT_DELAY_MS too small");
BUILD_ASSERT(TIMEDWAIT_TIMEOUT_MS >= 2 * TIMEDWAIT_TIMEOUT_DELAY_MS,
	     "TIMEDWAIT_TIMEOUT_MS too small");

static inline void timespec_add_ms(struct timespec *ts, uint32_t ms)
{
	struct timespec addend;

	timespec_from_timeout(K_MSEC(ms), &addend);
	timespec_add(ts, &addend);
}

/**
 * @brief Test to demonstrate limited condition variable resources
 *
 * @details Exactly SYS_THREAD_CONDVAR_MIN can be in use at once (when heap allocation is
 * unavailable).
 */
ZTEST(posix_threads_base, test_cond_resource_exhausted)
{
	posix_test_skip_if_native_libc();
	size_t i;
	pthread_cond_t m[SYS_THREAD_CONDVAR_MIN + 1];

	for (i = 0; i < SYS_THREAD_CONDVAR_MIN; ++i) {
		zassert_ok(pthread_cond_init(&m[i], NULL), "failed to init cond %zu", i);
	}

	/* try to initialize one more than SYS_THREAD_CONDVAR_MIN */
	zassert_equal(i, SYS_THREAD_CONDVAR_MIN);

	if (SYS_THREAD_CONDVAR_MIN == CONFIG_SYS_THREAD_CONDVAR_MAX) {
		/* This test may be removed eventally, since this assertion is successful only when
		 * heap allocation is unavailable, which is non-standard.
		 */
		zassert_not_equal(0, pthread_cond_init(&m[i], NULL),
				  "should not have initialized cond %zu", i);
	}

	for (; i > 0; --i) {
		zassert_ok(pthread_cond_destroy(&m[i - 1]), "failed to destroy cond %zu", i - 1);
	}
}

/**
 * @brief Test to that there are no condition variable resource leaks
 *
 * @details Demonstrate that condition variables may be used over and over again.
 */
ZTEST(posix_threads_base, test_cond_resource_leak)
{
	posix_test_skip_if_native_libc();
	pthread_cond_t cond;

	for (size_t i = 0; i < 2 * SYS_THREAD_CONDVAR_MIN; ++i) {
		zassert_ok(pthread_cond_init(&cond, NULL), "failed to init cond %zu", i);
		zassert_ok(pthread_cond_destroy(&cond), "failed to destroy cond %zu", i);
	}
}

static void test_pthread_condattr(void)
{
	pthread_condattr_t att = {0};

	zassert_ok(pthread_condattr_init(&att));

	zassert_ok(pthread_condattr_destroy(&att));
}

ZTEST_THREADS_BASE(test_pthread_condattr);

static void test_cond_init_existing_initialized_condattr(void)
{
	pthread_cond_t cond;
	pthread_condattr_t att = {0};

	zassert_ok(pthread_condattr_init(&att));
	zassert_ok(pthread_cond_init(&cond, &att), "pthread_cond_init failed with valid attr");

	/* Clean up */
	zassert_ok(pthread_cond_destroy(&cond));
	zassert_ok(pthread_condattr_destroy(&att));
}

ZTEST_THREADS_BASE(test_cond_init_existing_initialized_condattr);

static void test_cond_broadcast_static_init_pthread_cond_t(void)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	zassert_ok(pthread_cond_broadcast(&cond));
	zassert_ok(pthread_cond_destroy(&cond));
}

ZTEST_THREADS_BASE(test_cond_broadcast_static_init_pthread_cond_t);

static void test_cond_signal_static_init_pthread_cond_t(void)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	zassert_ok(pthread_cond_signal(&cond));
	zassert_ok(pthread_cond_destroy(&cond));
}

ZTEST_THREADS_BASE(test_cond_signal_static_init_pthread_cond_t);

static ZTEST_BMEM pthread_mutex_t cond_wait_mtx;
static ZTEST_BMEM pthread_cond_t cond_wait_cv;
static ZTEST_BMEM bool cond_wait_done;

static void *cond_wait_fn(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_mutex_lock(&cond_wait_mtx));
	while (!cond_wait_done) {
		zassert_ok(pthread_cond_wait(&cond_wait_cv, &cond_wait_mtx));
	}
	zassert_ok(pthread_mutex_unlock(&cond_wait_mtx));

	return NULL;
}

static void test_pthread_cond_wait(void)
{
	pthread_t th;

	posix_test_skip_if_native_libc();

	cond_wait_done = false;
	zassert_ok(pthread_mutex_init(&cond_wait_mtx, NULL));
	zassert_ok(pthread_cond_init(&cond_wait_cv, NULL));

	zassert_ok(pthread_create(&th, NULL, cond_wait_fn, NULL));

	k_msleep(TIMEDWAIT_TIMEOUT_DELAY_MS);

	zassert_ok(pthread_mutex_lock(&cond_wait_mtx));
	cond_wait_done = true;
	zassert_ok(pthread_cond_signal(&cond_wait_cv));
	zassert_ok(pthread_mutex_unlock(&cond_wait_mtx));

	zassert_ok(pthread_join(th, NULL));

	zassert_ok(pthread_cond_destroy(&cond_wait_cv));
	zassert_ok(pthread_mutex_destroy(&cond_wait_mtx));
}

ZTEST_THREADS_BASE(test_pthread_cond_wait);

static void *cond_timedwait_fn(void *arg)
{
	int ret;
	struct timespec time_point;
	pthread_mutex_t *mtx = (pthread_mutex_t *)arg;

	zassume_ok(clock_gettime(CLOCK_REALTIME, &time_point));
	timespec_add_ms(&time_point, TIMEDWAIT_TIMEOUT_MS);

	zassert_ok(pthread_mutex_lock(mtx));
	ret = pthread_cond_timedwait(&cond_wait_cv, mtx, &time_point);
	zassert_ok(pthread_mutex_unlock(mtx));

	return INT_TO_POINTER(ret);
}

static void test_pthread_cond_timedwait(void)
{
	void *ret;
	pthread_t th;

	posix_test_skip_if_native_libc();

	zassert_ok(pthread_mutex_init(&cond_wait_mtx, NULL));
	zassert_ok(pthread_cond_init(&cond_wait_cv, NULL));

	printk("Expecting timedwait with timeout of %d ms to fail\n", TIMEDWAIT_TIMEOUT_MS);
	zassert_ok(pthread_create(&th, NULL, cond_timedwait_fn, &cond_wait_mtx));
	zassert_ok(pthread_join(th, &ret));
	zassert_equal(ETIMEDOUT, POINTER_TO_INT(ret));

	printk("Expecting timedwait with timeout of %d ms to succeed after %d ms\n",
	       TIMEDWAIT_TIMEOUT_MS, TIMEDWAIT_TIMEOUT_DELAY_MS);
	zassert_ok(pthread_create(&th, NULL, cond_timedwait_fn, &cond_wait_mtx));
	k_msleep(TIMEDWAIT_TIMEOUT_DELAY_MS);
	zassert_ok(pthread_mutex_lock(&cond_wait_mtx));
	zassert_ok(pthread_cond_signal(&cond_wait_cv));
	zassert_ok(pthread_mutex_unlock(&cond_wait_mtx));
	zassert_ok(pthread_join(th, &ret));
	zassert_equal(0, POINTER_TO_INT(ret));

	IF_NOT_NATIVE_LIBC({
		zassert_ok(pthread_mutex_lock(&cond_wait_mtx));
		zassert_equal(EINVAL, pthread_cond_timedwait(&cond_wait_cv, &cond_wait_mtx, NULL));
		zassert_ok(pthread_mutex_unlock(&cond_wait_mtx));
	})

	zassert_ok(pthread_cond_destroy(&cond_wait_cv));
	zassert_ok(pthread_mutex_destroy(&cond_wait_mtx));
}

ZTEST_THREADS_BASE(test_pthread_cond_timedwait);

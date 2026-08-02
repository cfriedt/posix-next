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

#define SLEEP_MS 100

static ZTEST_BMEM pthread_mutex_t mutex;

static void *normal_mutex_entry(void *p1)
{
	int i, rc;

	ARG_UNUSED(p1);

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
 * @brief Uses the default mutex attributes. pthread_mutex_trylock and
 *	  pthread_mutex_lock are exercised without pthread_mutexattr_settype().
 */
static void mutex_lock_normal(void)
{
	pthread_t th;

	zassert_ok(pthread_mutex_init(&mutex, NULL));

	zassert_ok(pthread_mutex_lock(&mutex));

	zassert_ok(pthread_create(&th, NULL, normal_mutex_entry, NULL));

	k_msleep(SLEEP_MS);
	zassert_ok(pthread_mutex_unlock(&mutex));

	zassert_ok(pthread_join(th, NULL));

	zassert_ok(pthread_mutex_destroy(&mutex));
}

static void mutex_lock_static_init(void)
{
	pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

	zassert_ok(pthread_mutex_lock(&m));
	zassert_ok(pthread_mutex_unlock(&m));
	zassert_ok(pthread_mutex_destroy(&m));
}

static void test_pthread_mutex_lock(void)
{
	mutex_lock_static_init();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		mutex_lock_normal();
	}
}

ZTEST_THREADS_BASE(test_pthread_mutex_lock);

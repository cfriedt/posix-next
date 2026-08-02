/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "_main.h"

static ZTEST_BMEM pthread_mutex_t trylock_mutex;
static ZTEST_BMEM int trylock_status;

static void *trylock_fn(void *arg)
{
	ARG_UNUSED(arg);

	trylock_status = pthread_mutex_trylock(&trylock_mutex);

	return NULL;
}

static void test_pthread_mutex_trylock(void)
{
	pthread_t th;

	zassert_ok(pthread_mutex_init(&trylock_mutex, NULL));

	trylock_status = -1;
	zassert_ok(pthread_mutex_lock(&trylock_mutex));
	zassert_ok(pthread_create(&th, NULL, trylock_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
	zassert_equal(trylock_status, EBUSY, "expected EBUSY, got %d", trylock_status);
	zassert_ok(pthread_mutex_unlock(&trylock_mutex));

	zassert_ok(pthread_mutex_trylock(&trylock_mutex));
	zassert_ok(pthread_mutex_unlock(&trylock_mutex));

	zassert_ok(pthread_mutex_destroy(&trylock_mutex));
}

ZTEST_THREADS_BASE(test_pthread_mutex_trylock);

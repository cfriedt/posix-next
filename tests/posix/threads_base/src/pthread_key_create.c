/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static ZTEST_BMEM pthread_key_t dtor_key;
static ZTEST_BMEM pthread_key_t null_key;
static ZTEST_BMEM int dtor_calls;
static ZTEST_BMEM int null_dtor_calls;

static void store_again_dtor(void *value)
{
	++dtor_calls;
	(void)pthread_setspecific(dtor_key, value);
}

static void null_value_dtor(void *value)
{
	ARG_UNUSED(value);
	++null_dtor_calls;
}

static void *store_again_fn(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_setspecific(null_key, NULL));
	zassert_ok(pthread_setspecific(dtor_key, (void *)0x42));

	return NULL;
}

/*
 * A destructor is only called for a non-NULL value, and one that stores a value again is
 * called at least PTHREAD_DESTRUCTOR_ITERATIONS times before the thread gives up on it.
 */
static void key_create_destructor_iterations(void)
{
	pthread_t th;

	dtor_calls = 0;
	null_dtor_calls = 0;
	zassert_ok(pthread_key_create(&null_key, null_value_dtor));
	zassert_ok(pthread_key_create(&dtor_key, store_again_dtor));
	zassert_ok(pthread_create(&th, NULL, store_again_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
	zassert_equal(null_dtor_calls, 0, "destructor ran for a NULL value");
	zassert_true(dtor_calls >= PTHREAD_DESTRUCTOR_ITERATIONS, "%d destructor calls",
		     dtor_calls);
	zassert_ok(pthread_key_delete(dtor_key));
	zassert_ok(pthread_key_delete(null_key));
}

static void test_pthread_key_create(void)
{
	int ret;
	size_t n = 0;
	pthread_key_t k[CONFIG_POSIX_THREAD_KEYS_MAX + 1];

	posix_test_skip_if_native_libc();

	for (; n < ARRAY_SIZE(k); ++n) {
		ret = pthread_key_create(&k[n], NULL);
		if (ret != 0) {
			break;
		}
	}
	zassert_true(n < ARRAY_SIZE(k), "key pool did not exhaust");
	zassert_equal(ret, EAGAIN, "expected EAGAIN, got %d", ret);

	for (size_t i = 0; i < n; ++i) {
		zassert_ok(pthread_key_delete(k[i]));
	}

	key_create_destructor_iterations();
}

ZTEST_THREADS_BASE(test_pthread_key_create);

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void *getspecific_fn(void *arg)
{
	pthread_key_t *key = arg;

	zassert_ok(pthread_setspecific(*key, (void *)0x42));
	zassert_equal(pthread_getspecific(*key), (void *)0x42);

	return NULL;
}

static void test_pthread_getspecific(void)
{
	pthread_t th;
	pthread_key_t key;

	posix_test_skip_if_native_libc();

	zassert_ok(pthread_key_create(&key, NULL));
	zassert_ok(pthread_create(&th, NULL, getspecific_fn, &key));
	zassert_ok(pthread_join(th, NULL));
	zassert_ok(pthread_key_delete(key));

	zassert_is_null(pthread_getspecific(key));
	zassert_equal(pthread_setspecific(key, (void *)0x42), EINVAL);
}

ZTEST_THREADS_BASE(test_pthread_getspecific);

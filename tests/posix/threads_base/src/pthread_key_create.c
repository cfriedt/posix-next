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
}

ZTEST_THREADS_BASE(test_pthread_key_create);

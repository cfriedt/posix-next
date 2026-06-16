/*
 * Copyright (c) 2023, Harshil Bhatt
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define N_THR 3

static pthread_barrier_t barrier;
static int barrier_return[N_THR];
static int barrier_done[N_THR];

static void *barrier_thread(void *p1)
{
	int id = POINTER_TO_INT(p1);

	barrier_return[id] = pthread_barrier_wait(&barrier);
	barrier_done[id] = 1;

	return NULL;
}

ZTEST(posix_barriers, test_pthread_barrierattr_init)
{
	int ret, pshared;
	pthread_barrierattr_t attr;

	ret = pthread_barrierattr_init(&attr);
	zassert_equal(ret, 0, "pthread_barrierattr_init failed");

	ret = pthread_barrierattr_getpshared(&attr, &pshared);
	zassert_equal(ret, 0, "pthread_barrierattr_getpshared failed");
	zassert_equal(pshared, PTHREAD_PROCESS_PRIVATE, "pshared attribute not set correctly");

	ret = pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
	zassert_equal(ret, 0, "pthread_barrierattr_setpshared failed");

	ret = pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	zassert_equal(ret, 0, "pthread_barrierattr_setpshared failed");

	ret = pthread_barrierattr_getpshared(&attr, &pshared);
	zassert_equal(ret, 0, "pthread_barrierattr_getpshared failed");
	zassert_equal(pshared, PTHREAD_PROCESS_SHARED, "pshared attribute not retrieved correctly");

	ret = pthread_barrierattr_setpshared(&attr, 42);
	zassert_equal(ret, -EINVAL, "pthread_barrierattr_setpshared did not return EINVAL");

	ret = pthread_barrierattr_destroy(&attr);
	zassert_equal(ret, 0, "pthread_barrierattr_destroy failed");
}

ZTEST(posix_barriers, test_pthread_barrier_init)
{
	zassert_ok(pthread_barrier_init(&barrier, NULL, N_THR));
	zassert_ok(pthread_barrier_destroy(&barrier));
}

ZTEST(posix_barriers, test_pthread_barrier_wait)
{
	pthread_t threads[N_THR];
	int serial_threads = 0;
	int i;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	zassert_ok(pthread_barrier_init(&barrier, NULL, N_THR));

	for (i = 0; i < N_THR; i++) {
		barrier_done[i] = 0;
		barrier_return[i] = 0;
		zassert_ok(pthread_create(&threads[i], NULL, barrier_thread, INT_TO_POINTER(i)));
	}

	for (i = 0; i < N_THR; i++) {
		zassert_ok(pthread_join(threads[i], NULL));
		if (barrier_return[i] == PTHREAD_BARRIER_SERIAL_THREAD) {
			serial_threads++;
		}
	}

	zassert_equal(serial_threads, 1, "expected exactly one PTHREAD_BARRIER_SERIAL_THREAD");
	zassert_ok(pthread_barrier_destroy(&barrier));
}

ZTEST(posix_barriers, test_pthread_barrier_destroy)
{
	zassert_ok(pthread_barrier_init(&barrier, NULL, 2));
	zassert_ok(pthread_barrier_destroy(&barrier));
}

ZTEST_SUITE(posix_barriers, NULL, NULL, NULL, NULL, NULL);

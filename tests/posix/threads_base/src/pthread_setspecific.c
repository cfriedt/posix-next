/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "_main.h"

#define N_THR 2
#define N_KEY 2

static ZTEST_BMEM pthread_key_t keys[N_KEY];
static ZTEST_BMEM int alloc_count_t0;
static ZTEST_BMEM int alloc_count_t1;

static void *thread_top(void *p1)
{
	void *value = (void *)0x42;

	ARG_UNUSED(p1);

	zassert_ok(pthread_setspecific(keys[0], value), "pthread_setspecific failed");
	zassert_equal(pthread_getspecific(keys[0]), value,
		      "set and retrieved values are different");

	return NULL;
}

static void *thread_func(void *p1)
{
	void *value = (void *)0x73;

	ARG_UNUSED(p1);

	for (int i = 0; i < N_KEY; i++) {
		zassert_ok(pthread_setspecific(keys[i], value), "pthread_setspecific failed");
		zassert_equal(pthread_getspecific(keys[i]), value,
			      "set and retrieved values are different");
	}

	return NULL;
}

static void make_key(void)
{
	zassert_ok(pthread_key_create(&keys[0], NULL), "insufficient memory to create key");
}

static void make_keys(void)
{
	for (int i = 0; i < N_KEY; i++) {
		zassert_ok(pthread_key_create(&keys[i], NULL),
			   "insufficient memory to create keys");
	}
}

static void setspecific_key_1toN_thread(void)
{
	void *retval;
	pthread_t newthread[N_THR];

	make_key();

	for (int i = 0; i < N_THR; i++) {
		zassert_ok(pthread_create(&newthread[i], NULL, thread_top, NULL),
			   "attempt to create thread %d failed", i);
	}

	for (int i = 0; i < N_THR; i++) {
		zassert_ok(pthread_join(newthread[i], &retval), "failed to join thread %d", i);
	}

	zassert_ok(pthread_key_delete(keys[0]), "attempt to delete key failed");
}

static void setspecific_key_Nto1_thread(void)
{
	pthread_t newthread;

	make_keys();

	zassert_ok(pthread_create(&newthread, NULL, thread_func, NULL),
		   "attempt to create thread failed");

	zassert_ok(pthread_join(newthread, NULL), "failed to join thread");

	for (int i = 0; i < N_KEY; i++) {
		zassert_ok(pthread_key_delete(keys[i]), "attempt to delete keys[%d] failed", i);
	}
}

static ZTEST_BMEM pthread_key_t alloc_keys[CONFIG_POSIX_THREAD_KEYS_MAX];

static void *setspecific_thread(void *count)
{
	int value = 42;
	int *alloc_count = count;

	while (1) {
		pthread_key_t tkey;

		if (*alloc_count == CONFIG_POSIX_THREAD_KEYS_MAX / N_THR) {
			break;
		}

		if (pthread_key_create(&tkey, NULL) != 0) {
			break;
		}
		if (pthread_setspecific(tkey, &value) != 0) {
			break;
		}
		alloc_keys[*alloc_count] = tkey;
		*alloc_count += 1;
	}

	return NULL;
}

static void setspecific_delete_keys(int count)
{
	for (int i = 0; i < count; ++i) {
		zassert_ok(pthread_key_delete(alloc_keys[i]));
	}
}

static void setspecific_data_deallocation(void)
{
	pthread_t thread;

	alloc_count_t0 = 0;
	alloc_count_t1 = 0;

	zassert_ok(pthread_create(&thread, NULL, setspecific_thread, &alloc_count_t0),
		   "attempt to create thread failed");
	zassert_ok(pthread_join(thread, NULL), "failed to join thread");
	printk("first thread allocated %d keys\n", alloc_count_t0);
	setspecific_delete_keys(alloc_count_t0);

	zassert_ok(pthread_create(&thread, NULL, setspecific_thread, &alloc_count_t1),
		   "attempt to create thread failed");
	zassert_ok(pthread_join(thread, NULL), "failed to join thread");
	printk("second thread allocated %d keys\n", alloc_count_t1);
	setspecific_delete_keys(alloc_count_t1);

	zassert_equal(alloc_count_t0, alloc_count_t1,
		      "failed to deallocate thread specific data");
}

static void test_pthread_setspecific(void)
{
	setspecific_key_1toN_thread();
	setspecific_key_Nto1_thread();
	setspecific_data_deallocation();
}

ZTEST_THREADS_BASE(test_pthread_setspecific);

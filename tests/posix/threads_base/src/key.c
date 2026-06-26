/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "_main.h"

#define N_THR 2
#define N_KEY 2

static ZTEST_BMEM pthread_key_t key;
static ZTEST_BMEM pthread_key_t keys[N_KEY];
static ZTEST_BMEM pthread_once_t key_once;
static ZTEST_BMEM pthread_once_t keys_once;
static ZTEST_BMEM int alloc_count_t0;
static ZTEST_BMEM int alloc_count_t1;

static void key_test_reset(void)
{
	key_once = (pthread_once_t)PTHREAD_ONCE_INIT;
	keys_once = (pthread_once_t)PTHREAD_ONCE_INIT;
}

static void *thread_top(void *p1)
{
	void *value = (void *)0x42;

	ARG_UNUSED(p1);

	zassert_ok(pthread_setspecific(key, value), "pthread_setspecific failed");
	zassert_equal(pthread_getspecific(key), value, "set and retrieved values are different");

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
	zassert_ok(pthread_key_create(&key, NULL), "insufficient memory to create key");
}

static void make_keys(void)
{
	for (int i = 0; i < N_KEY; i++) {
		zassert_ok(pthread_key_create(&keys[i], NULL),
			   "insufficient memory to create keys");
	}
}

static void test_key_1toN_thread(void)
{
	void *retval;
	pthread_t newthread[N_THR];

	key_test_reset();
	zassert_ok(pthread_once(&key_once, make_key), "attempt to create key failed");

	for (int i = 0; i < N_THR; i++) {
		zassert_ok(pthread_create(&newthread[i], NULL, thread_top, NULL),
			   "attempt to create thread %d failed", i);
	}

	for (int i = 0; i < N_THR; i++) {
		zassert_ok(pthread_join(newthread[i], &retval), "failed to join thread %d", i);
	}

	zassert_ok(pthread_key_delete(key), "attempt to delete key failed");
}

static void test_key_Nto1_thread(void)
{
	pthread_t newthread;

	key_test_reset();
	zassert_ok(pthread_once(&keys_once, make_keys), "attempt to create keys failed");

	zassert_ok(pthread_create(&newthread, NULL, thread_func, NULL),
		   "attempt to create thread failed");

	zassert_ok(pthread_join(newthread, NULL), "failed to join thread");

	for (int i = 0; i < N_KEY; i++) {
		zassert_ok(pthread_key_delete(keys[i]), "attempt to delete keys[%d] failed", i);
	}
}

ZTEST_THREADS_BASE(test_key_1toN_thread);
ZTEST_THREADS_BASE(test_key_Nto1_thread);

ZTEST(posix_threads_base, test_key_resource_leak)
{
	pthread_key_t leak_key;

	for (size_t i = 0; i < CONFIG_POSIX_THREAD_KEYS_MAX; ++i) {
		zassert_ok(pthread_key_create(&leak_key, NULL), "failed to create key %zu", i);
		zassert_ok(pthread_key_delete(leak_key), "failed to delete key %zu", i);
	}
}

static void test_correct_key_is_deleted(void)
{
	pthread_key_t deleted_key;
	size_t j = CONFIG_POSIX_THREAD_KEYS_MAX - 1;
	pthread_key_t all_keys[CONFIG_POSIX_THREAD_KEYS_MAX];

	for (size_t i = 0; i < ARRAY_SIZE(all_keys); ++i) {
		zassert_ok(pthread_key_create(&all_keys[i], NULL), "failed to create key %zu", i);
	}

	deleted_key = all_keys[j];
	zassert_ok(pthread_key_delete(all_keys[j]));
	zassert_ok(pthread_key_create(&all_keys[j], NULL), "failed to create key %zu", j);

	zassert_equal(deleted_key, all_keys[j], "deleted key %x instead of key %x", all_keys[j],
		      deleted_key);

	for (size_t i = 0; i < ARRAY_SIZE(all_keys); ++i) {
		zassert_ok(pthread_key_delete(all_keys[i]), "failed to delete key %zu", i);
	}
}

ZTEST_THREADS_BASE(test_correct_key_is_deleted);

static void *setspecific_thread(void *count)
{
	int value = 42;
	int *alloc_count = count;

	while (1) {
		pthread_key_t tkey;

		if (*alloc_count == CONFIG_POSIX_THREAD_KEYS_MAX / N_THR) {
			break;
		}

		if (pthread_key_create(&tkey, NULL) < 0) {
			break;
		}
		if (pthread_setspecific(tkey, &value) == ENOMEM) {
			break;
		}
		*alloc_count += 1;
	}

	return NULL;
}

static void test_thread_specific_data_deallocation(void)
{
	pthread_t thread;

	alloc_count_t0 = 0;
	alloc_count_t1 = 0;

	zassert_ok(pthread_create(&thread, NULL, setspecific_thread, &alloc_count_t0),
		   "attempt to create thread failed");
	zassert_ok(pthread_join(thread, NULL), "failed to join thread");
	printk("first thread allocated %d keys\n", alloc_count_t0);

	zassert_ok(pthread_create(&thread, NULL, setspecific_thread, &alloc_count_t1),
		   "attempt to create thread failed");
	zassert_ok(pthread_join(thread, NULL), "failed to join thread");
	printk("second thread allocated %d keys\n", alloc_count_t1);

	zassert_equal(alloc_count_t0, alloc_count_t1,
		      "failed to deallocate thread specific data");
}

ZTEST_THREADS_BASE(test_thread_specific_data_deallocation);

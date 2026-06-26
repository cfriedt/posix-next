/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>

#include <zephyr/sys/atomic.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"
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

static void test_key_1toN_thread(void)
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

static void test_key_Nto1_thread(void)
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

	zassert_equal(deleted_key, all_keys[j], "deleted key %lx instead of key %lx", (long)all_keys[j],
		      (long)deleted_key);

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

static void test_pthread_key_create(void)
{
	int ret;
	size_t n = 0;
	pthread_key_t k[CONFIG_POSIX_THREAD_KEYS_MAX + 1];

	posix_test_skip_if_native_libc();

	/* exhausting the key pool fails with EAGAIN */
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

/* the entry runs in another thread, which cannot reach this one's stack under userspace */
static ZTEST_BMEM pthread_key_t getspecific_key;

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

	posix_test_skip_if_native_libc();

	zassert_ok(pthread_key_create(&getspecific_key, NULL));
	zassert_ok(pthread_create(&th, NULL, getspecific_fn, &getspecific_key));
	zassert_ok(pthread_join(th, NULL));
	zassert_ok(pthread_key_delete(getspecific_key));

	/* lookups against a deleted key fail closed */
	zassert_is_null(pthread_getspecific(getspecific_key));
	zassert_equal(pthread_setspecific(getspecific_key, (void *)0x42), EINVAL);
}

ZTEST_THREADS_BASE(test_pthread_getspecific);

static ZTEST_BMEM atomic_t key_delete_worker_ready;
static ZTEST_BMEM atomic_t key_delete_done;
static ZTEST_BMEM bool dtor_executed;

static void dtor_active_thread(void *arg)
{
	ARG_UNUSED(arg);
	dtor_executed = true;
}

static void *key_delete_worker(void *arg)
{
	pthread_key_t *key = arg;
	int val = 0x1234;

	zassert_ok(pthread_setspecific(*key, &val), "failed to set thread-specific data");

	/* Signal that data has been bound to the key */
	atomic_set(&key_delete_worker_ready, 1);

	/* Wait until key deletion completes before exiting */
	while (atomic_get(&key_delete_done) == 0) {
		sched_yield();
	}

	return NULL;
}

/**
 * @brief Verify pthread_key_delete() on key bound to active thread.
 *
 * Verifies that deleting a key while a thread holds thread-specific data
 * unlinks the key from the thread's internal list, suppresses destructor
 * invocation at thread exit per IEEE 1003.1, and does not cause double-free
 * or memory corruption.
 */
static void test_key_delete_active_thread(void)
{
	pthread_t thread;
	pthread_key_t key;

	dtor_executed = false;
	atomic_clear(&key_delete_worker_ready);
	atomic_clear(&key_delete_done);

	zassert_ok(pthread_key_create(&key, dtor_active_thread), "failed to create key");

	zassert_ok(pthread_create(&thread, NULL, key_delete_worker, &key),
		   "failed to create worker thread");

	/* Wait deterministically for worker thread to bind data */
	while (atomic_get(&key_delete_worker_ready) == 0) {
		sched_yield();
	}

	/* Delete key while worker thread is still alive and blocked */
	zassert_ok(pthread_key_delete(key), "failed to delete key");

	/* Release worker thread to exit and finalize */
	atomic_set(&key_delete_done, 1);

	/* Wait for worker thread to exit and finalize */
	zassert_ok(pthread_join(thread, NULL), "failed to join worker thread");

	/* Per IEEE 1003.1, destructor must not be called for deleted keys */
	zassert_false(dtor_executed, "destructor was executed for deleted key");
}

ZTEST_THREADS_BASE(test_key_delete_active_thread);

#define N_DTOR_KEY MIN(PTHREAD_DESTRUCTOR_ITERATIONS + 2, CONFIG_POSIX_THREAD_KEYS_MAX)

static ZTEST_BMEM pthread_key_t dtor_keys[N_DTOR_KEY];
static ZTEST_BMEM atomic_t dtor_calls;

static void dtor_count(void *arg)
{
	ARG_UNUSED(arg);
	atomic_inc(&dtor_calls);
}

static void *set_all_dtor_keys(void *arg)
{
	ARG_UNUSED(arg);

	for (size_t i = 0; i < N_DTOR_KEY; i++) {
		zassert_ok(pthread_setspecific(dtor_keys[i], &dtor_keys[i]));
	}

	return NULL;
}

/**
 * @brief Verify every key's destructor runs when a thread holds more keys than
 * PTHREAD_DESTRUCTOR_ITERATIONS.
 */
static void test_destructor_called_for_every_key(void)
{
	pthread_t thread;

	atomic_set(&dtor_calls, 0);
	for (size_t i = 0; i < N_DTOR_KEY; i++) {
		zassert_ok(pthread_key_create(&dtor_keys[i], dtor_count));
	}

	zassert_ok(pthread_create(&thread, NULL, set_all_dtor_keys, NULL));
	zassert_ok(pthread_join(thread, NULL));

	zassert_equal(atomic_get(&dtor_calls), N_DTOR_KEY);

	for (size_t i = 0; i < N_DTOR_KEY; i++) {
		zassert_ok(pthread_key_delete(dtor_keys[i]));
	}
}

ZTEST_THREADS_BASE(test_destructor_called_for_every_key);

static ZTEST_BMEM pthread_key_t resetting_key;

static void dtor_resetting(void *arg)
{
	atomic_inc(&dtor_calls);
	(void)pthread_setspecific(resetting_key, arg);
}

static void *set_resetting_key(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_setspecific(resetting_key, &resetting_key));

	return NULL;
}

/**
 * @brief Verify a destructor that keeps setting a value stops after
 * PTHREAD_DESTRUCTOR_ITERATIONS passes.
 */
static void test_destructor_iterations_bounded(void)
{
	pthread_t thread;

	atomic_set(&dtor_calls, 0);
	zassert_ok(pthread_key_create(&resetting_key, dtor_resetting));

	zassert_ok(pthread_create(&thread, NULL, set_resetting_key, NULL));
	zassert_ok(pthread_join(thread, NULL));

	zassert_equal(atomic_get(&dtor_calls), PTHREAD_DESTRUCTOR_ITERATIONS);

	zassert_ok(pthread_key_delete(resetting_key));
}

ZTEST_THREADS_BASE(test_destructor_iterations_bounded);

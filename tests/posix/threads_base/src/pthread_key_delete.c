/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "_main.h"

static ZTEST_BMEM pthread_key_t held_key;
/* sem_t is not usable from user mode yet: spin on shared flags */
static ZTEST_BMEM atomic_t held_ready;
static ZTEST_BMEM atomic_t release;
static ZTEST_BMEM int held_dtor_calls;

static void held_dtor(void *value)
{
	ARG_UNUSED(value);
	++held_dtor_calls;
}

static void *hold_key_value(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_setspecific(held_key, (void *)0x42));
	atomic_set(&held_ready, 1);
	while (atomic_get(&release) == 0) {
		sched_yield();
	}
	/* the key is gone, and so is this thread's value */
	zassert_equal(pthread_setspecific(held_key, (void *)0x73), EINVAL);
	zassert_is_null(pthread_getspecific(held_key));

	return NULL;
}

/* deleting a key drops the values other threads still hold, and runs no destructor for them */
static void key_delete_live_association(void)
{
	pthread_t th;

	held_dtor_calls = 0;
	atomic_clear(&held_ready);
	atomic_clear(&release);
	zassert_ok(pthread_key_create(&held_key, held_dtor));
	zassert_ok(pthread_create(&th, NULL, hold_key_value, NULL));
	while (atomic_get(&held_ready) == 0) {
		sched_yield();
	}
	zassert_ok(pthread_key_delete(held_key));
	atomic_set(&release, 1);
	zassert_ok(pthread_join(th, NULL));
	zassert_equal(held_dtor_calls, 0, "destructor ran for a deleted key");
}

/* the calling thread's own value goes the same way, and the key is free for reuse */
static void key_delete_own_association(void)
{
	pthread_key_t key, again;

	held_dtor_calls = 0;
	zassert_ok(pthread_key_create(&key, held_dtor));
	zassert_ok(pthread_setspecific(key, (void *)0x42));
	zassert_ok(pthread_key_delete(key));
	zassert_ok(pthread_key_create(&again, held_dtor));
	zassert_equal(key, again);
	zassert_is_null(pthread_getspecific(again));
	zassert_ok(pthread_key_delete(again));
	zassert_equal(held_dtor_calls, 0);
}

static void key_delete_no_leak(void)
{
	pthread_key_t leak_key;

	for (size_t i = 0; i < CONFIG_POSIX_THREAD_KEYS_MAX; ++i) {
		zassert_ok(pthread_key_create(&leak_key, NULL), "failed to create key %zu", i);
		zassert_ok(pthread_key_delete(leak_key), "failed to delete key %zu", i);
	}
}

static void key_delete_correct_key(void)
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

	zassert_equal(deleted_key, all_keys[j], "deleted key %lx instead of key %lx",
		      (long)all_keys[j], (long)deleted_key);

	for (size_t i = 0; i < ARRAY_SIZE(all_keys); ++i) {
		zassert_ok(pthread_key_delete(all_keys[i]), "failed to delete key %zu", i);
	}
}

static void test_pthread_key_delete(void)
{
	if (!k_is_user_context()) {
		key_delete_no_leak();
	}

	key_delete_correct_key();
	key_delete_live_association();
	key_delete_own_association();
}

ZTEST_THREADS_BASE(test_pthread_key_delete);

/*
 * Copyright (c) 2024, Meta
 * Copyright (c) 2024, Marvin Ouma <pancakesdeath@protonmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

ZTEST_BMEM bool attr_valid;
ZTEST_BMEM pthread_attr_t attr;
#ifndef CONFIG_NATIVE_LIBC
const pthread_attr_t uninit_attr;
#endif

static ZTEST_BMEM bool detached_thread_has_finished;

static void *thread_entry(void *arg)
{
	bool joinable = (bool)POINTER_TO_UINT(arg);

	if (!joinable) {
		detached_thread_has_finished = true;
	}

	return NULL;
}

void create_thread_common_entry(const pthread_attr_t *attrp, bool expect_success, bool joinable,
				void *(*entry)(void *arg), void *arg)
{
	pthread_t th;

	if (!joinable) {
		detached_thread_has_finished = false;
	}

	if (expect_success) {
		zassert_ok(pthread_create(&th, attrp, entry, arg));
	} else {
		zassert_not_ok(pthread_create(&th, attrp, entry, arg));
		return;
	}

	if (joinable) {
		zassert_ok(pthread_join(th, NULL), "failed to join joinable thread");
		return;
	}

	/*
	 * Should not be able to join a detached thread. POSIX leaves this
	 * undefined; Zephyr detects it and fails the join, but ASAN's
	 * pthread_join interceptor aborts on it before the implementation
	 * can return.
	 */
	if (!IS_ENABLED(CONFIG_ASAN)) {
		zassert_not_ok(pthread_join(th, NULL));
	}

	/* bounded by real time: under parallel load sim time can lag far behind */
	for (uint32_t t0 = k_uptime_get_32(); !detached_thread_has_finished && (k_uptime_get_32() - t0) < 5000;) {
		k_msleep(2 * CONFIG_PTHREAD_RECYCLER_DELAY_MS);
	}

	zassert_true(detached_thread_has_finished, "detached thread did not seem to finish");
}

void can_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common_entry(attrp, true, true, thread_entry, UINT_TO_POINTER(true));
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_attr_init(&attr));
	attr_valid = true;
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	if (attr_valid) {
		(void)pthread_attr_destroy(&attr);
		attr_valid = false;
	}
}

ZTEST_SUITE(xsi_realtime_threads, NULL, NULL, before, after, NULL);

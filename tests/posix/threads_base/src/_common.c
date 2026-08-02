/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../shared/linux_compat_test.h"
#include "_main.h"

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

ZTEST_BMEM pthread_attr_t test_attr;
ZTEST_BMEM bool test_attr_valid;
const pthread_attr_t uninit_attr;
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
	for (uint32_t t0 = now_ms(); !detached_thread_has_finished && (now_ms() - t0) < 5000;) {
		msleep(2 * CONFIG_PTHREAD_RECYCLER_DELAY_MS);
	}

	zassert_true(detached_thread_has_finished, "detached thread did not seem to finish");
}

void create_thread_common(const pthread_attr_t *attrp, bool expect_success, bool joinable)
{
	create_thread_common_entry(attrp, expect_success, joinable, thread_entry,
				   UINT_TO_POINTER(joinable));
}

void can_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, true, true);
}

void cannot_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, false, true);
}

void pthread_attr_before(void *fixture)
{
	ARG_UNUSED(fixture);

	zassert_ok(pthread_attr_init(&test_attr));
	test_attr_valid = true;
}

void pthread_attr_after(void *fixture)
{
	ARG_UNUSED(fixture);

	if (test_attr_valid) {
		(void)pthread_attr_destroy(&test_attr);
		test_attr_valid = false;
	}
}

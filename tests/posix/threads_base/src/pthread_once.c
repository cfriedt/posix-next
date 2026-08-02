/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static ZTEST_BMEM pthread_once_t once_control;
static ZTEST_BMEM int once_count;
static ZTEST_BMEM int once_entries;
static ZTEST_BMEM bool once_started;

static void once_func(void)
{
	++once_count;
}

static void once_slow(void)
{
	once_started = true;
	msleep(20);
	++once_count;
}

/* the first entry blocks at a cancellation point until cancelled; the retry completes */
static void once_cancellable(void)
{
	once_started = true;
	if (once_entries++ == 0) {
		zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL));
		while (true) {
			pthread_testcancel();
			msleep(1);
		}
	}
	++once_count;
}

static void *once_fn(void *arg)
{
	zassert_ok(pthread_once(&once_control, (void (*)(void))arg));

	return NULL;
}

static void once_reset(void)
{
	once_control = (pthread_once_t)PTHREAD_ONCE_INIT;
	once_count = 0;
	once_entries = 0;
	once_started = false;
}

static void once_await_started(void)
{
	while (!once_started) {
		msleep(1);
	}
}

static void pthread_once_single(void)
{
	pthread_t th;

	once_reset();

	zassert_ok(pthread_once(&once_control, once_func));
	zassert_ok(pthread_once(&once_control, once_func));

	zassert_ok(pthread_create(&th, NULL, once_fn, once_func));
	zassert_ok(pthread_join(th, NULL));

	zassert_equal(once_count, 1, "init routine ran %d times", once_count);
}

static void pthread_once_contended(void)
{
	pthread_t th;

	once_reset();

	zassert_ok(pthread_create(&th, NULL, once_fn, once_slow));
	once_await_started();

	/* must block until the initializer in th completes */
	zassert_ok(pthread_once(&once_control, once_slow));
	zassert_equal(once_count, 1, "returned before init routine completed");

	zassert_ok(pthread_join(th, NULL));
	zassert_equal(once_count, 1, "init routine ran %d times", once_count);
}

static void pthread_once_cancelled(void)
{
	pthread_t initializer;
	pthread_t waiter;
	void *result;

	once_reset();

	zassert_ok(pthread_create(&initializer, NULL, once_fn, once_cancellable));
	once_await_started();

	zassert_ok(pthread_create(&waiter, NULL, once_fn, once_cancellable));
	msleep(10);

	/* cancelling the initializer resets once_control; the waiter re-runs the init routine */
	zassert_ok(pthread_cancel(initializer));
	zassert_ok(pthread_join(initializer, &result));
	zassert_equal(result, PTHREAD_CANCELED);
	zassert_ok(pthread_join(waiter, NULL));

	zassert_equal(once_entries, 2, "init routine entered %d times", once_entries);
	zassert_equal(once_count, 1, "init routine completed %d times", once_count);

	zassert_ok(pthread_once(&once_control, once_cancellable));
	zassert_equal(once_entries, 2, "init routine re-ran after completion");
}

static void test_pthread_once(void)
{
	pthread_once_single();
	pthread_once_contended();
	pthread_once_cancelled();
}

ZTEST_THREADS_BASE(test_pthread_once);

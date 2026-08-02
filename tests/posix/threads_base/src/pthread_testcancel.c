/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static ZTEST_BMEM bool testcancel_ignored;
static ZTEST_BMEM bool testcancel_failed;

static void *testcancel_fn(void *arg)
{
	ARG_UNUSED(arg);

	if (!k_is_user_context()) {
		/* kernel threads must explicitly unmask cancel (not a POSIX sigaddset signo) */
		zassert_ok(k_sig_addset(&k_current_get()->base.sig.mask, K_SIG_CANCEL));
	}

	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));

	testcancel_ignored = false;

	/* queue cancellation while disabled */
	zassert_ok(pthread_cancel(pthread_self()));

	/* cancellation point with no pending delivery while disabled */
	pthread_testcancel();

	testcancel_ignored = true;

	testcancel_failed = false;

	/* enable the thread to be cancelled, the thread should not return */
	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL));

	/* intentionally sleep to encounter cancellation point */
	msleep(20);

	testcancel_failed = true;

	return NULL;
}

static void test_pthread_testcancel(void)
{
	pthread_t th;

	zassert_ok(pthread_create(&th, NULL, testcancel_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
	zassert_true(testcancel_ignored);
	zassert_false(testcancel_failed);
}

ZTEST_THREADS_BASE(test_pthread_testcancel);

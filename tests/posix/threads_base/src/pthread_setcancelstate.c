/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "_main.h"

#define PTHREAD_CANCEL_INVALID -1

static void *setcancelstate_fn(void *arg)
{
	int oldstate = -1;

	ARG_UNUSED(arg);

	zassert_equal(pthread_setcancelstate(PTHREAD_CANCEL_INVALID, NULL), EINVAL);

	/* no initial-state expectation: kernel-mode threads start with cancel masked */
	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL));
	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate));
	zassert_equal(oldstate, PTHREAD_CANCEL_DISABLE);
	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate));
	zassert_equal(oldstate, PTHREAD_CANCEL_ENABLE);
	zassert_ok(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL));

	return NULL;
}

static void test_pthread_setcancelstate(void)
{
	pthread_t th;

	zassert_ok(pthread_create(&th, NULL, setcancelstate_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
}

ZTEST_THREADS_BASE(test_pthread_setcancelstate);

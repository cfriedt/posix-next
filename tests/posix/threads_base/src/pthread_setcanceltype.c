/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>

#include <zephyr/ztest.h>

#include "_main.h"

#define PTHREAD_CANCEL_INVALID -1

static void *setcanceltype_fn(void *arg)
{
	int oldtype = -1;

	ARG_UNUSED(arg);

	zassert_equal(pthread_setcanceltype(PTHREAD_CANCEL_INVALID, NULL), EINVAL);

	zassert_ok(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype));
	zassert_equal(oldtype, PTHREAD_CANCEL_DEFERRED);
	zassert_ok(pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype));
	zassert_equal(oldtype, PTHREAD_CANCEL_ASYNCHRONOUS);

	return NULL;
}

static void test_pthread_setcanceltype(void)
{
	pthread_t th;

	zassert_ok(pthread_create(&th, NULL, setcanceltype_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
}

ZTEST_THREADS_BASE(test_pthread_setcanceltype);

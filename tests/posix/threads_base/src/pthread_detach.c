/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <stdbool.h>

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static ZTEST_BMEM bool detached_fn_done;

static void *detached_fn(void *arg)
{
	ARG_UNUSED(arg);

	msleep(20);
	detached_fn_done = true;

	return NULL;
}

static void test_pthread_detach(void)
{
	pthread_t th;

	detached_fn_done = false;
	zassert_ok(pthread_create(&th, NULL, detached_fn, NULL));
	zassert_ok(pthread_detach(th));

	for (size_t i = 0; i < 100 && !detached_fn_done; ++i) {
		msleep(20);
	}
	zassert_true(detached_fn_done, "detached thread did not finish");
}

ZTEST_THREADS_BASE(test_pthread_detach);

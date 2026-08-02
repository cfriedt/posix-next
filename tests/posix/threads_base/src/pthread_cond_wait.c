/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

#define COND_WAIT_DELAY_MS 100

static ZTEST_BMEM pthread_mutex_t cond_wait_mtx;
static ZTEST_BMEM pthread_cond_t cond_wait_cv;
static ZTEST_BMEM bool cond_wait_done;

static void *cond_wait_fn(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_mutex_lock(&cond_wait_mtx));
	while (!cond_wait_done) {
		zassert_ok(pthread_cond_wait(&cond_wait_cv, &cond_wait_mtx));
	}
	zassert_ok(pthread_mutex_unlock(&cond_wait_mtx));

	return NULL;
}

static void test_pthread_cond_wait(void)
{
	pthread_t th;

	posix_test_skip_if_native_libc();

	cond_wait_done = false;
	zassert_ok(pthread_mutex_init(&cond_wait_mtx, NULL));
	zassert_ok(pthread_cond_init(&cond_wait_cv, NULL));

	zassert_ok(pthread_create(&th, NULL, cond_wait_fn, NULL));

	k_msleep(COND_WAIT_DELAY_MS);

	zassert_ok(pthread_mutex_lock(&cond_wait_mtx));
	cond_wait_done = true;
	zassert_ok(pthread_cond_signal(&cond_wait_cv));
	zassert_ok(pthread_mutex_unlock(&cond_wait_mtx));

	zassert_ok(pthread_join(th, NULL));

	zassert_ok(pthread_cond_destroy(&cond_wait_cv));
	zassert_ok(pthread_mutex_destroy(&cond_wait_mtx));
}

ZTEST_THREADS_BASE(test_pthread_cond_wait);

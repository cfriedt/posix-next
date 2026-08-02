/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "_main.h"

static void *pthread_create_fn(void *arg)
{
	ARG_UNUSED(arg);

	return NULL;
}

static void pthread_create_null_attr(void)
{
	create_thread_common(NULL, true, true);
}

static void pthread_create_descriptor_leak(void)
{
	pthread_t pthread1;

	/* If we are leaking descriptors, then this loop will never complete */
	for (size_t i = 0; i < CONFIG_POSIX_THREAD_THREADS_MAX * 2; ++i) {
		zassert_ok(pthread_create(&pthread1, NULL, pthread_create_fn, NULL),
			   "unable to create thread %zu", i);
		zassert_ok(pthread_join(pthread1, NULL), "unable to join thread %zu", i);
	}
}

#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
static void pthread_create_explicit_sched(void)
{
	pthread_t th;
	pthread_attr_t eattr;
	struct sched_param param;

	zassert_ok(pthread_attr_init(&eattr));
	zassert_ok(pthread_attr_getschedparam(&eattr, &param));
	zassert_ok(pthread_attr_setinheritsched(&eattr, PTHREAD_EXPLICIT_SCHED));
	zassert_ok(pthread_attr_setschedparam(&eattr, &param));
	zassert_ok(pthread_create(&th, &eattr, pthread_create_fn, NULL));
	zassert_ok(pthread_join(th, NULL));
	zassert_ok(pthread_attr_destroy(&eattr));
}
#endif

static void test_pthread_create(void)
{
	pthread_create_null_attr();

	if (!k_is_user_context()) {
		pthread_create_descriptor_leak();
	}

#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		pthread_create_explicit_sched();
	}
#endif
}

ZTEST_THREADS_BASE(test_pthread_create);

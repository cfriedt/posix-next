/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

ZTEST(posix_thread_sporadic_server, test_pthread_attr_setschedpolicy)
{
	int policy;
	pthread_attr_t attr;

	zassert_ok(pthread_attr_init(&attr));
	zassert_ok(pthread_attr_setschedpolicy(&attr, SCHED_SPORADIC));
	zassert_ok(pthread_attr_getschedpolicy(&attr, &policy));
	zassert_equal(policy, SCHED_SPORADIC);
	zassert_ok(pthread_attr_destroy(&attr));
}

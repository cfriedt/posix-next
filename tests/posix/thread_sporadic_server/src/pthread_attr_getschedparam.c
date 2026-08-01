/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

#include "thread_sporadic_server_tests.h"

ZTEST(posix_thread_sporadic_server, test_pthread_attr_getschedparam)
{
	pthread_attr_t attr;
	int priority = sched_get_priority_max(SCHED_SPORADIC);
	struct sched_param param = sporadic_param(priority);
	struct sched_param readback = {0};

	zassert_ok(pthread_attr_init(&attr));
	zassert_ok(pthread_attr_setschedpolicy(&attr, SCHED_SPORADIC));
	zassert_ok(pthread_attr_setschedparam(&attr, &param));
	zassert_ok(pthread_attr_getschedparam(&attr, &readback));
	zassert_equal(readback.sched_priority, priority);
	zassert_ok(pthread_attr_destroy(&attr));
}

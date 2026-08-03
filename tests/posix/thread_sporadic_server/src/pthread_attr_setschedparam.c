/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

#include "thread_sporadic_server_tests.h"

ZTEST_USER(posix_thread_sporadic_server, test_pthread_attr_setschedparam)
{
	pthread_attr_t attr;
	struct sched_param param = sporadic_param(sched_get_priority_min(SCHED_SPORADIC));

	zassert_ok(pthread_attr_init(&attr));
	zassert_ok(pthread_attr_setschedpolicy(&attr, SCHED_SPORADIC));
	zassert_ok(pthread_attr_setschedparam(&attr, &param));

	param.sched_ss_max_repl = 0;
	zassert_equal(pthread_attr_setschedparam(&attr, &param), EINVAL);
	param.sched_ss_max_repl = _POSIX_SS_REPL_MAX + 1;
	zassert_equal(pthread_attr_setschedparam(&attr, &param), EINVAL);

	zassert_ok(pthread_attr_destroy(&attr));
}

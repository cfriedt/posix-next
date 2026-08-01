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

ZTEST(posix_thread_sporadic_server, test_pthread_setschedparam)
{
	int policy;
	struct sched_param orig;
	struct sched_param param = sporadic_param(sched_get_priority_min(SCHED_SPORADIC));

	zassert_ok(pthread_getschedparam(pthread_self(), &policy, &orig));

	zassert_ok(pthread_setschedparam(pthread_self(), SCHED_SPORADIC, &param));

	param.sched_ss_max_repl = 0;
	zassert_equal(pthread_setschedparam(pthread_self(), SCHED_SPORADIC, &param), EINVAL);
	param.sched_ss_max_repl = _POSIX_SS_REPL_MAX + 1;
	zassert_equal(pthread_setschedparam(pthread_self(), SCHED_SPORADIC, &param), EINVAL);

	zassert_ok(pthread_setschedparam(pthread_self(), policy, &orig));
}

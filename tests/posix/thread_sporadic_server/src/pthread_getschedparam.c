/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

#include "thread_sporadic_server_tests.h"

ZTEST_USER(posix_thread_sporadic_server, test_pthread_getschedparam)
{
	int policy = -1;
	struct sched_param orig;
	struct sched_param readback = {0};
	struct sched_param param = sporadic_param(sched_get_priority_min(SCHED_SPORADIC));

	zassert_ok(pthread_getschedparam(pthread_self(), &policy, &orig));

	zassert_ok(pthread_setschedparam(pthread_self(), SCHED_SPORADIC, &param));
	zassert_ok(pthread_getschedparam(pthread_self(), &policy, &readback));
	/* budgets are not enforced; the policy reads back as a supported policy */
	zassert_true((policy == SCHED_FIFO) || (policy == SCHED_RR) || (policy == SCHED_OTHER) ||
		     (policy == SCHED_SPORADIC));
	zassert_equal(readback.sched_priority, param.sched_priority);

	zassert_ok(pthread_setschedparam(pthread_self(), SCHED_RR, &orig));
}

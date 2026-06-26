/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include <zephyr/ztest.h>

#include "../../common/linux_compat_test.h"
#include "_main.h"

#ifndef CONFIG_NATIVE_LIBC
/* Zephyr SDK 1.0.1 does not include prototypes for pthread_kill() */
int pthread_kill(pthread_t thread, int sig);
#endif

void test_pthread_sigmask_init_expectation_mask(void);
void test_pthread_sigmask_common(int (*sigmask)(int how, const sigset_t *set, sigset_t *oset));

static void test_pthread_sigmask(void)
{
	test_pthread_sigmask_common(pthread_sigmask);
}

static void test_pthread_kill(void)
{
	zassert_ok(pthread_kill(pthread_self(), 0));
	zassert_equal(pthread_kill(pthread_self(), -1), EINVAL);
	IF_NOT_NATIVE_LIBC({
		/* glibc does not return ESRCH for attempting to kill a non-existent thread (POSIX
		 * non-conformance)
		 */
		zassert_equal(pthread_kill((pthread_t)0, SIGUSR1), ESRCH);
	})
}

ZTEST_THREADS_BASE(test_pthread_sigmask);
ZTEST_THREADS_BASE(test_pthread_kill);

void pthread_signal_setup(void)
{
	test_pthread_sigmask_init_expectation_mask();
}

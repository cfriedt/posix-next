/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <signal.h>

#include <zephyr/ztest.h>

#include "_main.h"

void test_pthread_sigmask_init_expectation_mask(void);
void test_pthread_sigmask_common(int (*sigmask)(int how, const sigset_t *set, sigset_t *oset));

static void test_pthread_sigmask(void)
{
	test_pthread_sigmask_common(pthread_sigmask);
}

ZTEST_THREADS_BASE(test_pthread_sigmask);

void pthread_signal_setup(void)
{
	test_pthread_sigmask_init_expectation_mask();
}

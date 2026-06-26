/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THREADS_BASE_MAIN_H_
#define THREADS_BASE_MAIN_H_

#include <stddef.h>

#include <zephyr/ztest.h>

typedef void (*posix_threads_base_setup_fn)(void);
typedef void (*posix_threads_base_hook_fn)(void *fixture);

struct posix_threads_base_module {
	posix_threads_base_setup_fn setup;
	posix_threads_base_hook_fn before;
	posix_threads_base_hook_fn after;
	posix_threads_base_hook_fn teardown;
};

#ifdef CONFIG_USERSPACE
#define ZTEST_THREADS_BASE_USER(fn) \
	ZTEST_USER(posix_threads_base, fn##_user) { fn(); }
#else
#define ZTEST_THREADS_BASE_USER(fn)
#endif

#define ZTEST_THREADS_BASE(fn)  \
	ZTEST(posix_threads_base, fn) \
	{                             \
		fn();                 \
	}                             \
	ZTEST_THREADS_BASE_USER(fn)

void mutex_before(void *fixture);
void pthread_attr_before(void *fixture);
void pthread_attr_after(void *fixture);
void pthread_signal_setup(void);

#endif /* THREADS_BASE_MAIN_H_ */

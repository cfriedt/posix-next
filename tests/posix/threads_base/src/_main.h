/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THREADS_BASE_MAIN_H_
#define THREADS_BASE_MAIN_H_

#include <pthread.h>
#include <stdbool.h>
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

void pthread_attr_before(void *fixture);
void pthread_attr_after(void *fixture);
void pthread_signal_setup(void);

/* shared thread-creation fixture and helpers (_common.c) */
extern pthread_attr_t test_attr;
extern bool test_attr_valid;
extern const pthread_attr_t uninit_attr;

void create_thread_common_entry(const pthread_attr_t *attrp, bool expect_success, bool joinable,
				void *(*entry)(void *arg), void *arg);
void create_thread_common(const pthread_attr_t *attrp, bool expect_success, bool joinable);
void can_create_thread(const pthread_attr_t *attrp);
void cannot_create_thread(const pthread_attr_t *attrp);

#endif /* THREADS_BASE_MAIN_H_ */

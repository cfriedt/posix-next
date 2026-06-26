/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../common/linux_compat_test.h"
#include "_main.h"

#include <pthread.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define BIOS_FOOD           0xB105F00D
#define INVALID_DETACHSTATE 7373

static ZTEST_BMEM bool attr_valid;
static ZTEST_BMEM pthread_attr_t attr;
__maybe_unused static const pthread_attr_t uninit_attr;
static ZTEST_BMEM bool detached_thread_has_finished;

static void *thread_entry(void *arg)
{
	bool joinable = (bool)POINTER_TO_UINT(arg);

	if (!joinable) {
		detached_thread_has_finished = true;
	}

	return NULL;
}

static void create_thread_common_entry(const pthread_attr_t *attrp, bool expect_success,
				       bool joinable, void *(*entry)(void *arg), void *arg)
{
	pthread_t th;

	if (!joinable) {
		detached_thread_has_finished = false;
	}

	if (expect_success) {
		zassert_ok(pthread_create(&th, attrp, entry, arg));
	} else {
		zassert_not_ok(pthread_create(&th, attrp, entry, arg));
		return;
	}

	if (joinable) {
		zassert_ok(pthread_join(th, NULL), "failed to join joinable thread");
		return;
	}

	/* should not be able to join detached thread */
	zassert_not_ok(pthread_join(th, NULL));

	for (size_t i = 0; i < 10; ++i) {
		msleep(2 * CONFIG_PTHREAD_RECYCLER_DELAY_MS);
		if (detached_thread_has_finished) {
			break;
		}
	}

	zassert_true(detached_thread_has_finished, "detached thread did not seem to finish");
}

static void create_thread_common(const pthread_attr_t *attrp, bool expect_success, bool joinable)
{
	create_thread_common_entry(attrp, expect_success, joinable, thread_entry,
				   UINT_TO_POINTER(joinable));
}

static inline void can_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, true, true);
}

static inline void cannot_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, false, true);
}

static void test_null_attr(void)
{
	create_thread_common(NULL, true, true);
}

ZTEST_THREADS_BASE(test_null_attr);

static void test_pthread_attr_init_destroy(void)
{
	/* attr has already been initialized in before() */

	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_ok(pthread_attr_init(&attr));
			zassert_equal(pthread_attr_destroy((pthread_attr_t *)&uninit_attr), EINVAL);
		}
	})

	can_create_thread(&attr);

	/* can destroy an initialized attr */
	zassert_ok(pthread_attr_destroy(&attr), "failed to destroy an initialized attr");
	attr_valid = false;

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* glibc does not return EINVAL passing an invalid attribute to pthread_create()
		 * (POSIX non-conformance)
		 */
		cannot_create_thread(&attr);
	}

	if (false) {
		/* undefined behaviour */
		zassert_ok(pthread_attr_destroy(&attr));
	}

	/* can re-initialize a destroyed attr */
	zassert_ok(pthread_attr_init(&attr));
	attr_valid = true;

	can_create_thread(&attr);

	/* note: attr is still valid and is destroyed in after() */
}

ZTEST_THREADS_BASE(test_pthread_attr_init_destroy);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
static void test_pthread_attr_getschedparam(void)
{
	struct sched_param param = {
		.sched_priority = BIOS_FOOD,
	};

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getschedparam(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getschedparam(NULL, &param), EINVAL);
			zassert_equal(pthread_attr_getschedparam(&uninit_attr, &param), EINVAL);
		}
		zassert_equal(pthread_attr_getschedparam(&attr, NULL), EINVAL);
	})

	/* only check to see that the function succeeds and sets param */
	zassert_ok(pthread_attr_getschedparam(&attr, &param));
	zassert_not_equal(BIOS_FOOD, param.sched_priority);
}

ZTEST_THREADS_BASE(test_pthread_attr_getschedparam);

static void test_pthread_attr_setschedparam(void)
{
	struct sched_param param = {0};

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setschedparam(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_setschedparam(NULL, &param), EINVAL);
			zassert_equal(
				pthread_attr_setschedparam((pthread_attr_t *)&uninit_attr, &param),
				EINVAL);
		}
		/* avoid glibc non-null compiler warning promoted to error */
		zassert_equal(pthread_attr_setschedparam(&attr, NULL), EINVAL);
	})

	zassert_ok(pthread_attr_setschedparam(&attr, &param));

	can_create_thread(&attr);
}

ZTEST_THREADS_BASE(test_pthread_attr_setschedparam);

#endif

static void test_pthread_attr_getdetachstate(void)
{
	int detachstate;

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getdetachstate(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getdetachstate(NULL, &detachstate), EINVAL);
			zassert_equal(pthread_attr_getdetachstate(&uninit_attr, &detachstate),
				      EINVAL);
		}
		/* avoid glibc non-null compiler warning promoted to error */
		zassert_equal(pthread_attr_getdetachstate(&attr, NULL), EINVAL);
	})

	/* default detachstate is joinable */
	zassert_ok(pthread_attr_getdetachstate(&attr, &detachstate));
	zassert_equal(detachstate, PTHREAD_CREATE_JOINABLE);
	can_create_thread(&attr);
}

ZTEST_THREADS_BASE(test_pthread_attr_getdetachstate);

static void test_pthread_attr_setdetachstate(void)
{
	int detachstate = PTHREAD_CREATE_JOINABLE;

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setdetachstate(NULL, INVALID_DETACHSTATE),
				      EINVAL);
			zassert_equal(pthread_attr_setdetachstate(NULL, detachstate), EINVAL);
			zassert_equal(pthread_attr_setdetachstate((pthread_attr_t *)&uninit_attr,
								  detachstate),
				      EINVAL);
		}
		/* glibc does not return EINVAL setting an invalid detachstate (POSIX
		 * non-conformance)
		 */
		zassert_equal(pthread_attr_setdetachstate(&attr, INVALID_DETACHSTATE), EINVAL);
	})

	/* read back detachstate just written */
	zassert_ok(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));
	zassert_ok(pthread_attr_getdetachstate(&attr, &detachstate));
	zassert_equal(detachstate, PTHREAD_CREATE_DETACHED);
	create_thread_common(&attr, true, false);
}

ZTEST_THREADS_BASE(test_pthread_attr_setdetachstate);

void pthread_attr_before(void *fixture)
{
	ARG_UNUSED(fixture);

	zassert_ok(pthread_attr_init(&attr));
	attr_valid = true;
	detached_thread_has_finished = false;
}

void pthread_attr_after(void *fixture)
{
	ARG_UNUSED(fixture);

	if (attr_valid) {
		(void)pthread_attr_destroy(&attr);
		attr_valid = false;
	}
}

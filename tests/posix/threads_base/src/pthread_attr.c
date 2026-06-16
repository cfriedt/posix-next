/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define BIOS_FOOD           0xB105F00D
#define INVALID_DETACHSTATE 7373

static bool attr_valid;
static pthread_attr_t attr;
static const pthread_attr_t uninit_attr;
static bool detached_thread_has_finished;

/* TODO: this should be optional */
#define STATIC_THREAD_STACK_SIZE (MAX(1024, K_KERNEL_STACK_LEN(0) + CONFIG_TEST_EXTRA_STACK_SIZE))
static K_THREAD_STACK_DEFINE(static_thread_stack, STATIC_THREAD_STACK_SIZE);

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

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		/* Most of this testsuite uses automatic stack allocation, but
		 * threads_base.static_stack uses statically allocated stacks.
		 */
		ztest_test_skip();
	}

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
		k_msleep(2 * CONFIG_PTHREAD_RECYCLER_DELAY_MS);
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

ZTEST(pthread_attr, test_null_attr)
{
	/*
	 * This test can only succeed when it is possible to call pthread_create() with a NULL
	 * pthread_attr_t* (I.e. when we have the ability to allocate thread stacks dynamically).
	 */
	create_thread_common(NULL, CONFIG_SYS_THREAD_STACK_MAX > 0, true);
}

ZTEST(pthread_attr, test_pthread_attr_init_destroy)
{
	/* attr has already been initialized in before() */

	if (false) {
		/* undefined behaviour */
		zassert_ok(pthread_attr_init(&attr));
	}

	/* cannot destroy an uninitialized attr */
	zassert_equal(pthread_attr_destroy((pthread_attr_t *)&uninit_attr), EINVAL);

	can_create_thread(&attr);

	/* can destroy an initialized attr */
	zassert_ok(pthread_attr_destroy(&attr), "failed to destroy an initialized attr");
	attr_valid = false;

	cannot_create_thread(&attr);

	if (false) {
		/* undefined behaviour */
		zassert_ok(pthread_attr_destroy(&attr));
	}

	/* can re-initialize a destroyed attr */
	zassert_ok(pthread_attr_init(&attr));
	/* TODO: pthread_attr_init() should be sufficient to initialize a thread by itself */
	zassert_ok(pthread_attr_setstack(&attr, &static_thread_stack, STATIC_THREAD_STACK_SIZE));
	attr_valid = true;

	can_create_thread(&attr);

	/* note: attr is still valid and is destroyed in after() */
}

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
ZTEST(pthread_attr, test_pthread_attr_getschedparam)
{
	struct sched_param param = {
		.sched_priority = BIOS_FOOD,
	};

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getschedparam(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getschedparam(NULL, &param), EINVAL);
			zassert_equal(pthread_attr_getschedparam(&uninit_attr, &param), EINVAL);
		}
		zassert_equal(pthread_attr_getschedparam(&attr, NULL), EINVAL);
	}

	/* only check to see that the function succeeds and sets param */
	zassert_ok(pthread_attr_getschedparam(&attr, &param));
	zassert_not_equal(BIOS_FOOD, param.sched_priority);
}

ZTEST(pthread_attr, test_pthread_attr_setschedparam)
{
	struct sched_param param = {0};

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setschedparam(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_setschedparam(NULL, &param), EINVAL);
			zassert_equal(
				pthread_attr_setschedparam((pthread_attr_t *)&uninit_attr, &param),
				EINVAL);
		}
		zassert_equal(pthread_attr_setschedparam(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_setschedparam(&attr, &param));

	can_create_thread(&attr);
}

#endif

ZTEST(pthread_attr, test_pthread_attr_getdetachstate)
{
	int detachstate;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getdetachstate(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getdetachstate(NULL, &detachstate), EINVAL);
			zassert_equal(pthread_attr_getdetachstate(&uninit_attr, &detachstate),
				      EINVAL);
		}
		zassert_equal(pthread_attr_getdetachstate(&attr, NULL), EINVAL);
	}

	/* default detachstate is joinable */
	zassert_ok(pthread_attr_getdetachstate(&attr, &detachstate));
	zassert_equal(detachstate, PTHREAD_CREATE_JOINABLE);
	can_create_thread(&attr);
}

ZTEST(pthread_attr, test_pthread_attr_setdetachstate)
{
	int detachstate = PTHREAD_CREATE_JOINABLE;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setdetachstate(NULL, INVALID_DETACHSTATE),
				      EINVAL);
			zassert_equal(pthread_attr_setdetachstate(NULL, detachstate), EINVAL);
			zassert_equal(pthread_attr_setdetachstate((pthread_attr_t *)&uninit_attr,
								  detachstate),
				      EINVAL);
		}
		zassert_equal(pthread_attr_setdetachstate(&attr, INVALID_DETACHSTATE), EINVAL);
	}

	/* read back detachstate just written */
	zassert_ok(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));
	zassert_ok(pthread_attr_getdetachstate(&attr, &detachstate));
	zassert_equal(detachstate, PTHREAD_CREATE_DETACHED);
	create_thread_common(&attr, true, false);
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_attr_init(&attr));
	attr_valid = true;
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	if (attr_valid) {
		(void)pthread_attr_destroy(&attr);
		attr_valid = false;
	}
}

ZTEST_SUITE(pthread_attr, NULL, NULL, before, after, NULL);

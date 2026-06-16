/*
 * Copyright (c) 2024, Meta
 * Copyright (c) 2024, Marvin Ouma <pancakesdeath@protonmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define BIOS_FOOD 0xB105F00D

static bool attr_valid;
static pthread_attr_t attr;
static const pthread_attr_t uninit_attr;
static bool detached_thread_has_finished;

#define STATIC_THREAD_STACK_SIZE (MAX(1024, PTHREAD_STACK_MIN + CONFIG_TEST_EXTRA_STACK_SIZE))
static K_THREAD_STACK_DEFINE(static_thread_stack, STATIC_THREAD_STACK_SIZE);
#define LARGE_THREAD_STACK_SIZE  (2 * STATIC_THREAD_STACK_SIZE)
static K_THREAD_STACK_DEFINE(large_thread_stack, LARGE_THREAD_STACK_SIZE);

static void reinit_thread_attr(void)
{
	zassert_ok(pthread_attr_destroy(&attr));
	attr_valid = false;
	zassert_ok(pthread_attr_init(&attr));
	attr_valid = true;
}

static void *thread_entry(void *arg)
{
	bool joinable = (bool)POINTER_TO_UINT(arg);

	if (!joinable) {
		detached_thread_has_finished = true;
	}

	return NULL;
}

static void create_thread_common(const pthread_attr_t *attrp, bool expect_success, bool joinable)
{
	pthread_t th;

	if (!joinable) {
		detached_thread_has_finished = false;
	}

	if (expect_success) {
		zassert_ok(pthread_create(&th, attrp, thread_entry, UINT_TO_POINTER(joinable)));
	} else {
		zassert_not_ok(pthread_create(&th, attrp, thread_entry, UINT_TO_POINTER(joinable)));
		return;
	}

	if (joinable) {
		zassert_ok(pthread_join(th, NULL), "failed to join joinable thread");
		return;
	}

	zassert_not_ok(pthread_join(th, NULL));

	for (size_t i = 0; i < 10; ++i) {
		k_msleep(2 * CONFIG_PTHREAD_RECYCLER_DELAY_MS);
		if (detached_thread_has_finished) {
			break;
		}
	}

	zassert_true(detached_thread_has_finished, "detached thread did not seem to finish");
}

static void can_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, true, true);
}

static inline void cannot_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, false, true);
}

ZTEST(xsi_threads_ext, test_pthread_attr_getstack)
{
	void *stackaddr = (void *)BIOS_FOOD;
	size_t stacksize = BIOS_FOOD;

	{
		if (false) {
			zassert_equal(pthread_attr_getstack(NULL, NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getstack(NULL, NULL, &stacksize), EINVAL);
			zassert_equal(pthread_attr_getstack(NULL, &stackaddr, NULL), EINVAL);
			zassert_equal(pthread_attr_getstack(NULL, &stackaddr, &stacksize), EINVAL);
			zassert_equal(pthread_attr_getstack(&uninit_attr, &stackaddr, &stacksize),
				      EINVAL);
		}
		zassert_equal(pthread_attr_getstack(&attr, NULL, NULL), EINVAL);
		zassert_equal(pthread_attr_getstack(&attr, NULL, &stacksize), EINVAL);
		zassert_equal(pthread_attr_getstack(&attr, &stackaddr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getstack(&attr, &stackaddr, &stacksize));
	zassert_not_equal(stackaddr, (void *)BIOS_FOOD);
	zassert_not_equal(stacksize, BIOS_FOOD);
}

ZTEST(xsi_threads_ext, test_pthread_attr_setstack)
{
	{
		if (false) {
			zassert_equal(pthread_attr_setstack(NULL, NULL, 0), EINVAL);
			zassert_equal(pthread_attr_setstack(NULL, NULL, PTHREAD_STACK_MIN), EINVAL);
			zassert_equal(pthread_attr_setstack(NULL, static_thread_stack, 0), EINVAL);
			zassert_equal(pthread_attr_setstack(NULL, static_thread_stack,
				STATIC_THREAD_STACK_SIZE), EINVAL);
			zassert_equal(pthread_attr_setstack((pthread_attr_t *)&uninit_attr,
			static_thread_stack, STATIC_THREAD_STACK_SIZE),
				      EINVAL);
		}
		zassert_equal(pthread_attr_setstack(&attr, NULL, 0), EINVAL);
		zassert_equal(pthread_attr_setstack(&attr, NULL, STATIC_THREAD_STACK_SIZE), EINVAL);
		zassert_equal(pthread_attr_setstack(&attr, static_thread_stack, 0), EINVAL);
	}

	/* can create a thread with the stack set up in before() */
	can_create_thread(&attr);

	reinit_thread_attr();
	zassert_ok(pthread_attr_setstack(&attr, large_thread_stack, LARGE_THREAD_STACK_SIZE));
	can_create_thread(&attr);
}

ZTEST(xsi_threads_ext, test_pthread_attr_getstacksize)
{
	size_t stacksize = BIOS_FOOD;

	{
		if (false) {
			zassert_equal(pthread_attr_getstacksize(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getstacksize(NULL, &stacksize), EINVAL);
			zassert_equal(pthread_attr_getstacksize(&uninit_attr, &stacksize), EINVAL);
		}
		zassert_equal(pthread_attr_getstacksize(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getstacksize(&attr, &stacksize));
	zassert_not_equal(stacksize, BIOS_FOOD);
}

ZTEST(xsi_threads_ext, test_pthread_attr_setstacksize)
{
	size_t stacksize;

	{
		if (false) {
			zassert_equal(pthread_attr_setstacksize(NULL, 0), EINVAL);
			zassert_equal(pthread_attr_setstacksize(NULL, PTHREAD_STACK_MIN), EINVAL);
			zassert_equal(pthread_attr_setstacksize((pthread_attr_t *)&uninit_attr,
								PTHREAD_STACK_MIN),
				      EINVAL);
		}
		zassert_equal(pthread_attr_setstacksize(&attr, 0), EINVAL);
	}

	can_create_thread(&attr);

	reinit_thread_attr();

	zassert_ok(pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN));
	zassert_ok(pthread_attr_getstacksize(&attr, &stacksize));
	zassert_true(stacksize >= PTHREAD_STACK_MIN);
	can_create_thread(&attr);
}

ZTEST(xsi_threads_ext, test_pthread_set_get_concurrency)
{
	zassert_equal(EINVAL, pthread_setconcurrency(-42));
	zassert_equal(0, pthread_getconcurrency());

	for (int i = 0; i <= CONFIG_MP_MAX_NUM_CPUS; ++i) {
		zassert_ok(pthread_setconcurrency(i));
		zassert_equal(i, pthread_getconcurrency());
	}

	zassert_equal(EAGAIN, pthread_setconcurrency(CONFIG_MP_MAX_NUM_CPUS + 1));
}

ZTEST(xsi_threads_ext, test_pthread_attr_static_corner_cases)
{
	pthread_attr_t attr1;

	if (CONFIG_SYS_THREAD_STACK_MAX > 0) {
		ztest_test_skip();
	}

	cannot_create_thread(NULL);

	zassert_ok(pthread_attr_init(&attr1));
	cannot_create_thread(&attr1);
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_attr_init(&attr));
	zassert_ok(pthread_attr_setstack(&attr, &static_thread_stack, STATIC_THREAD_STACK_SIZE));
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

ZTEST_SUITE(xsi_threads_ext, NULL, NULL, before, after, NULL);

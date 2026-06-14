/*
 * Copyright (c) 2024, Meta
 * Copyright (c) 2024, Marvin Ouma <pancakesdeath@protonmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define BIOS_FOOD     0xB105F00D
#define SCHED_INVALID 4242
#define PRIO_INVALID  -1

static bool attr_valid;
static pthread_attr_t attr;
static const pthread_attr_t uninit_attr;
static bool detached_thread_has_finished;

/*
 * This should be discarded by the linker, in this specific testsuite, if
 * CONFIG_DYNAMIC_THREAD_ALLOC is not set
 */
#define STATIC_THREAD_STACK_SIZE (MAX(1024, PTHREAD_STACK_MIN + CONFIG_TEST_EXTRA_STACK_SIZE))
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

static void can_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, true, true);
}

ZTEST(xsi_threads_ext, test_pthread_attr_getstack)
{
	void *stackaddr = (void *)BIOS_FOOD;
	size_t stacksize = BIOS_FOOD;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
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
	void *stackaddr;
	size_t stacksize;
	void *new_stackaddr;
	size_t new_stacksize;

	/* valid values */
	zassert_ok(pthread_attr_getstack(&attr, &stackaddr, &stacksize));

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setstack(NULL, NULL, 0), EINVAL);
			zassert_equal(pthread_attr_setstack(NULL, NULL, stacksize), EINVAL);
			zassert_equal(pthread_attr_setstack(NULL, stackaddr, 0), EINVAL);
			zassert_equal(pthread_attr_setstack(NULL, stackaddr, stacksize), EINVAL);
			zassert_equal(pthread_attr_setstack((pthread_attr_t *)&uninit_attr,
							    stackaddr, stacksize),
				      EINVAL);
		}
		zassert_equal(pthread_attr_setstack(&attr, NULL, 0), EINVAL);
		zassert_equal(pthread_attr_setstack(&attr, NULL, stacksize), EINVAL);
		zassert_equal(pthread_attr_setstack(&attr, stackaddr, 0), EINVAL);
	}

	/* ensure we can create and join a thread with the default attrs */
	can_create_thread(&attr);

	/* set stack / addr to the current values of stack / addr */
	zassert_ok(pthread_attr_setstack(&attr, stackaddr, stacksize));
	can_create_thread(&attr);

	/* qemu_x86 seems to be unable to set thread stacks to be anything less than 4096 */
	if (!IS_ENABLED(CONFIG_X86)) {
		/*
		 * check we can set a smaller stacksize
		 * should not require dynamic reallocation
		 * size may get rounded up to some alignment internally
		 */
		zassert_ok(pthread_attr_setstack(&attr, stackaddr, stacksize - 1));
		/* ensure we read back the same values as we specified */
		zassert_ok(pthread_attr_getstack(&attr, &new_stackaddr, &new_stacksize));
		zassert_equal(new_stackaddr, stackaddr);
		zassert_equal(new_stacksize, stacksize - 1);
		can_create_thread(&attr);
	}

	if (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
		/* ensure we can set a dynamic stack */
		k_thread_stack_t *stack;

		stack = k_thread_stack_alloc(2 * stacksize, 0);
		zassert_not_null(stack);

		zassert_ok(pthread_attr_setstack(&attr, (void *)stack, 2 * stacksize));
		/* ensure we read back the same values as we specified */
		zassert_ok(pthread_attr_getstack(&attr, &new_stackaddr, &new_stacksize));
		zassert_equal(new_stackaddr, (void *)stack);
		zassert_equal(new_stacksize, 2 * stacksize);
		can_create_thread(&attr);
	}
}

ZTEST(xsi_threads_ext, test_pthread_set_get_concurrency)
{
	/* EINVAL if the value specified by new_level is negative */
	zassert_equal(EINVAL, pthread_setconcurrency(-42));

	/*
	 * Note: the special value 0 indicates the implementation will
	 * maintain the concurrency level at its own discretion.
	 *
	 * pthread_getconcurrency() should return a value of 0 on init.
	 */
	zassert_equal(0, pthread_getconcurrency());

	for (int i = 0; i <= CONFIG_MP_MAX_NUM_CPUS; ++i) {
		zassert_ok(pthread_setconcurrency(i));
		/* verify parameter is saved */
		zassert_equal(i, pthread_getconcurrency());
	}

	/* EAGAIN if the a system resource to be exceeded */
	zassert_equal(EAGAIN, pthread_setconcurrency(CONFIG_MP_MAX_NUM_CPUS + 1));
}

ZTEST(xsi_threads_ext, test_pthread_attr_getstacksize)
{
	size_t stacksize = BIOS_FOOD;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
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
	size_t new_stacksize;

	/* valid size */
	zassert_ok(pthread_attr_getstacksize(&attr, &stacksize));

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setstacksize(NULL, 0), EINVAL);
			zassert_equal(pthread_attr_setstacksize(NULL, stacksize), EINVAL);
			zassert_equal(pthread_attr_setstacksize((pthread_attr_t *)&uninit_attr,
								stacksize),
				      EINVAL);
		}
		zassert_equal(pthread_attr_setstacksize(&attr, 0), EINVAL);
	}

	/* ensure we can spin up a thread with the default stack size */
	can_create_thread(&attr);

	/* set stack / addr to the current values of stack / addr */
	zassert_ok(pthread_attr_setstacksize(&attr, stacksize));
	/* ensure we can read back the values we just set */
	zassert_ok(pthread_attr_getstacksize(&attr, &new_stacksize));
	zassert_equal(new_stacksize, stacksize);
	can_create_thread(&attr);

	/* qemu_x86 seems to be unable to set thread stacks to be anything less than 4096 */
	if (!IS_ENABLED(CONFIG_X86)) {
		zassert_ok(pthread_attr_setstacksize(&attr, stacksize - 1));
		/* ensure we can read back the values we just set */
		zassert_ok(pthread_attr_getstacksize(&attr, &new_stacksize));
		zassert_equal(new_stacksize, stacksize - 1);
		can_create_thread(&attr);
	}

	if (IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
		zassert_ok(pthread_attr_setstacksize(&attr, 2 * stacksize));
		/* ensure we read back the same values as we specified */
		zassert_ok(pthread_attr_getstacksize(&attr, &new_stacksize));
		zassert_equal(new_stacksize, 2 * stacksize);
		can_create_thread(&attr);
	}
}

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)

ZTEST(xsi_threads_ext, test_pthread_attr_getschedpolicy)
{
	int policy = BIOS_FOOD;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getschedpolicy(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getschedpolicy(NULL, &policy), EINVAL);
			zassert_equal(pthread_attr_getschedpolicy(&uninit_attr, &policy), EINVAL);
		}
		zassert_equal(pthread_attr_getschedpolicy(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getschedpolicy(&attr, &policy));
	zassert_not_equal(BIOS_FOOD, policy);
}

ZTEST(xsi_threads_ext, test_pthread_attr_setschedpolicy)
{
	int policy = SCHED_OTHER;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setschedpolicy(NULL, SCHED_INVALID), EINVAL);
			zassert_equal(pthread_attr_setschedpolicy(NULL, policy), EINVAL);
			zassert_equal(
				pthread_attr_setschedpolicy((pthread_attr_t *)&uninit_attr, policy),
				EINVAL);
		}
		zassert_equal(pthread_attr_setschedpolicy(&attr, SCHED_INVALID), EINVAL);
	}

	zassert_ok(pthread_attr_setschedpolicy(&attr, SCHED_OTHER));
	policy = SCHED_INVALID;
	zassert_ok(pthread_attr_getschedpolicy(&attr, &policy));
	zassert_equal(policy, SCHED_OTHER);

	can_create_thread(&attr);
}

ZTEST(xsi_threads_ext, test_pthread_attr_getscope)
{
	int contentionscope = BIOS_FOOD;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getscope(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getscope(NULL, &contentionscope), EINVAL);
			zassert_equal(pthread_attr_getscope(&uninit_attr, &contentionscope),
				      EINVAL);
		}
		zassert_equal(pthread_attr_getscope(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getscope(&attr, &contentionscope));
	zassert_equal(contentionscope, PTHREAD_SCOPE_SYSTEM);
}

ZTEST(xsi_threads_ext, test_pthread_attr_setscope)
{
	int contentionscope = BIOS_FOOD;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setscope(NULL, PTHREAD_SCOPE_SYSTEM), EINVAL);
			zassert_equal(pthread_attr_setscope(NULL, contentionscope), EINVAL);
			zassert_equal(pthread_attr_setscope((pthread_attr_t *)&uninit_attr,
							    contentionscope),
				      EINVAL);
		}
		zassert_equal(pthread_attr_setscope(&attr, 3), EINVAL);
	}

	zassert_equal(pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS), ENOTSUP);
	zassert_ok(pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM));
	zassert_ok(pthread_attr_getscope(&attr, &contentionscope));
	zassert_equal(contentionscope, PTHREAD_SCOPE_SYSTEM);
}

ZTEST(xsi_threads_ext, test_pthread_attr_getinheritsched)
{
	int inheritsched = BIOS_FOOD;

	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_getinheritsched(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getinheritsched(NULL, &inheritsched), EINVAL);
			zassert_equal(pthread_attr_getinheritsched(&uninit_attr, &inheritsched),
				      EINVAL);
		}
		zassert_equal(pthread_attr_getinheritsched(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getinheritsched(&attr, &inheritsched));
	zassert_equal(inheritsched, PTHREAD_INHERIT_SCHED);
}

static void *inheritsched_entry(void *arg)
{
	int prio;
	int inheritsched;
	int pprio = POINTER_TO_INT(arg);

	zassert_ok(pthread_attr_getinheritsched(&attr, &inheritsched));

	prio = k_thread_priority_get(k_current_get());

	if (inheritsched == PTHREAD_INHERIT_SCHED) {
		zassert_equal(prio, pprio, "actual priority: %d, expected priority: %d", prio,
			      pprio);
		return NULL;
	}

	int act_prio;
	int exp_prio;
	int act_policy;
	int exp_policy;
	struct sched_param param;

	zassert_ok(pthread_getschedparam(pthread_self(), &act_policy, &param));
	act_prio = param.sched_priority;

	zassert_ok(pthread_attr_getschedpolicy(&attr, &exp_policy));
	zassert_ok(pthread_attr_getschedparam(&attr, &param));
	exp_prio = param.sched_priority;

	zassert_equal(act_policy, exp_policy, "actual policy: %d, expected policy: %d", act_policy,
		      exp_policy);
	zassert_equal(act_prio, exp_prio, "actual priority: %d, expected priority: %d", act_prio,
		      exp_prio);

	return NULL;
}

static void test_pthread_attr_setinheritsched_common(bool inheritsched)
{
	int prio;
	int policy;
	struct sched_param param;

	extern int zephyr_to_posix_priority(int priority, int *policy);

	prio = k_thread_priority_get(k_current_get());
	zassert_not_equal(prio, K_LOWEST_APPLICATION_THREAD_PRIO);

	prio = K_LOWEST_APPLICATION_THREAD_PRIO;
	param.sched_priority = zephyr_to_posix_priority(prio, &policy);

	zassert_ok(pthread_attr_setschedpolicy(&attr, policy));
	zassert_ok(pthread_attr_setschedparam(&attr, &param));
	zassert_ok(pthread_attr_setinheritsched(&attr, inheritsched));
	create_thread_common_entry(&attr, true, true, inheritsched_entry,
				   UINT_TO_POINTER(k_thread_priority_get(k_current_get())));
}

ZTEST(xsi_threads_ext, test_pthread_attr_setinheritsched)
{
	/* degenerate cases */
	{
		if (false) {
			/* undefined behaviour */
			zassert_equal(pthread_attr_setinheritsched(NULL, PTHREAD_EXPLICIT_SCHED),
				      EINVAL);
			zassert_equal(pthread_attr_setinheritsched(NULL, PTHREAD_INHERIT_SCHED),
				      EINVAL);
			zassert_equal(pthread_attr_setinheritsched((pthread_attr_t *)&uninit_attr,
								   PTHREAD_INHERIT_SCHED),
				      EINVAL);
		}
		zassert_equal(pthread_attr_setinheritsched(&attr, 3), EINVAL);
	}

	test_pthread_attr_setinheritsched_common(PTHREAD_INHERIT_SCHED);
	test_pthread_attr_setinheritsched_common(PTHREAD_EXPLICIT_SCHED);
}

ZTEST(xsi_threads_ext, test_pthread_setschedprio)
{
	int policy;
	int prio = 0;
	struct sched_param param;
	pthread_t self = pthread_self();

	zassert_equal(pthread_setschedprio(self, PRIO_INVALID), EINVAL, "EINVAL was expected");

	zassert_ok(pthread_setschedprio(self, prio));
	param.sched_priority = ~prio;
	zassert_ok(pthread_getschedparam(self, &policy, &param));
	zassert_equal(param.sched_priority, prio, "Priority unchanged");
}

#endif /* _POSIX_THREAD_PRIORITY_SCHEDULING */

int zephyr_to_posix_priority(int z_prio, int *policy);
int posix_to_zephyr_priority(int priority, int policy);

ZTEST(xsi_threads_ext, test_pthread_priority_conversion)
{
	for (int z_prio = -CONFIG_NUM_COOP_PRIORITIES, prio = CONFIG_NUM_COOP_PRIORITIES - 1,
		 p_prio, policy;
	     z_prio <= -1; z_prio++, prio--) {
		p_prio = zephyr_to_posix_priority(z_prio, &policy);
		zassert_equal(policy, SCHED_FIFO);
		zassert_equal(p_prio, prio, "%d %d\n", p_prio, prio);
		zassert_equal(z_prio, posix_to_zephyr_priority(p_prio, SCHED_FIFO));
	}

	for (int z_prio = 0, prio = CONFIG_NUM_PREEMPT_PRIORITIES - 1, p_prio, policy;
	     z_prio < CONFIG_NUM_PREEMPT_PRIORITIES; z_prio++, prio--) {
		p_prio = zephyr_to_posix_priority(z_prio, &policy);
		zassert_equal(policy, SCHED_RR);
		zassert_equal(p_prio, prio, "%d %d\n", p_prio, prio);
		zassert_equal(z_prio, posix_to_zephyr_priority(p_prio, SCHED_RR));
	}
}

static inline void cannot_create_thread(const pthread_attr_t *attrp)
{
	create_thread_common(attrp, false, true);
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

ZTEST(xsi_threads_ext, test_pthread_attr_large_stacksize)
{
	if (IS_ENABLED(CONFIG_COVERAGE)) {
		ztest_test_skip();
	}

	size_t actual_size;
	const size_t expect_size = 2 * CONFIG_SYS_THREAD_STACK_SIZE;

	zassert_ok(pthread_attr_setstacksize(&attr, expect_size));
	zassert_ok(pthread_attr_getstacksize(&attr, &actual_size));
	zassert_equal(actual_size, expect_size);

	if (CONFIG_SYS_THREAD_STACK_MAX > SYS_THREAD_STACK_MIN) {
		if (IS_ENABLED(CONFIG_SYS_THREAD_STACK_ALLOC_HEAP) &&
		    (K_HEAP_MEM_POOL_SIZE < 2 * K_THREAD_STACK_LEN(CONFIG_SYS_THREAD_STACK_SIZE))) {
			ztest_test_skip();
		}

		can_create_thread(&attr);
	}
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	zassert_ok(pthread_attr_init(&attr));
	if (!IS_ENABLED(CONFIG_DYNAMIC_THREAD_ALLOC)) {
		zassert_ok(pthread_attr_setstack(&attr, &static_thread_stack,
						 STATIC_THREAD_STACK_SIZE));
	}
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

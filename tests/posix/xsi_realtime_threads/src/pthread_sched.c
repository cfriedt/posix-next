/*
 * Copyright (c) 2024, Meta
 * Copyright (c) 2024, Marvin Ouma <pancakesdeath@protonmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>
#include <sched.h>

#include <zephyr/ztest.h>

#define BIOS_FOOD     0xB105F00D
#define SCHED_INVALID 4242
#define PRIO_INVALID  -1

extern bool attr_valid;
extern pthread_attr_t attr;
extern const pthread_attr_t uninit_attr;

extern void can_create_thread(const pthread_attr_t *attrp);
extern void create_thread_common_entry(const pthread_attr_t *attrp, bool expect_success,
				       bool joinable, void *(*entry)(void *arg), void *arg);

int zephyr_to_posix_priority(int z_prio, int *policy);
int posix_to_zephyr_priority(int priority, int policy);

ZTEST(xsi_realtime_threads, test_pthread_attr_getschedpolicy)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int policy = BIOS_FOOD;

	{
		if (false) {
			zassert_equal(pthread_attr_getschedpolicy(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getschedpolicy(NULL, &policy), EINVAL);
			zassert_equal(pthread_attr_getschedpolicy(&uninit_attr, &policy), EINVAL);
		}
		zassert_equal(pthread_attr_getschedpolicy(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getschedpolicy(&attr, &policy));
	zassert_not_equal(BIOS_FOOD, policy);
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_attr_setschedpolicy)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int policy = SCHED_OTHER;

	{
		if (false) {
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
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_attr_getscope)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int contentionscope = BIOS_FOOD;

	{
		if (false) {
			zassert_equal(pthread_attr_getscope(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getscope(NULL, &contentionscope), EINVAL);
			zassert_equal(pthread_attr_getscope(&uninit_attr, &contentionscope),
				      EINVAL);
		}
		zassert_equal(pthread_attr_getscope(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getscope(&attr, &contentionscope));
	zassert_equal(contentionscope, PTHREAD_SCOPE_SYSTEM);
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_attr_setscope)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int contentionscope = BIOS_FOOD;

	{
		if (false) {
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
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_attr_getinheritsched)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int inheritsched = BIOS_FOOD;

	{
		if (false) {
			zassert_equal(pthread_attr_getinheritsched(NULL, NULL), EINVAL);
			zassert_equal(pthread_attr_getinheritsched(NULL, &inheritsched), EINVAL);
			zassert_equal(pthread_attr_getinheritsched(&uninit_attr, &inheritsched),
				      EINVAL);
		}
		zassert_equal(pthread_attr_getinheritsched(&attr, NULL), EINVAL);
	}

	zassert_ok(pthread_attr_getinheritsched(&attr, &inheritsched));
	zassert_equal(inheritsched, PTHREAD_INHERIT_SCHED);
#else
	ztest_test_skip();
#endif
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

ZTEST(xsi_realtime_threads, test_pthread_attr_setinheritsched)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	{
		if (false) {
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
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_setschedprio)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int policy;
	int prio = 0;
	struct sched_param param;
	pthread_t self = pthread_self();

	zassert_equal(pthread_setschedprio(self, PRIO_INVALID), EINVAL, "EINVAL was expected");

	zassert_ok(pthread_setschedprio(self, prio));
	param.sched_priority = ~prio;
	zassert_ok(pthread_getschedparam(self, &policy, &param));
	zassert_equal(param.sched_priority, prio, "Priority unchanged");
#else
	ztest_test_skip();
#endif
}

ZTEST(xsi_realtime_threads, test_pthread_priority_conversion)
{
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
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
#else
	ztest_test_skip();
#endif
}

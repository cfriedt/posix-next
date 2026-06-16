/*
 * Copyright (c) 2024, Meta
 * Copyright (c) 2025 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <sched.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define SCHED_INVALID 4242

static pthread_attr_t attr;

static void *thread_entry(void *arg)
{
	ARG_UNUSED(arg);
	return NULL;
}

static void can_create_thread(const pthread_attr_t *attrp)
{
	pthread_t th;

	if (CONFIG_SYS_THREAD_STACK_MAX == 0) {
		ztest_test_skip();
	}

	zassert_ok(pthread_create(&th, attrp, thread_entry, NULL));
	zassert_ok(pthread_join(th, NULL));
}

ZTEST(xsi_realtime, test_sched_get_priority_min)
{
	errno = 0;
	zassert_equal(sched_get_priority_min(SCHED_INVALID), -1);
	zassert_equal(errno, EINVAL);
}

ZTEST(xsi_realtime, test_sched_get_priority_max)
{
	errno = 0;
	zassert_equal(sched_get_priority_max(SCHED_INVALID), -1);
	zassert_equal(errno, EINVAL);
}

ZTEST(xsi_realtime, test_sched_policy_and_priority_limits)
{
	int pmin = -1;
	int pmax = -1;
	struct sched_param param;
	static const int policies[] = {
		SCHED_FIFO,
		SCHED_RR,
		SCHED_OTHER,
		SCHED_INVALID,
	};
	static const char *const policy_names[] = {
		"SCHED_FIFO",
		"SCHED_RR",
		"SCHED_OTHER",
		"SCHED_INVALID",
	};
	static const bool policy_enabled[] = {
		CONFIG_NUM_COOP_PRIORITIES > 0,
		CONFIG_NUM_PREEMPT_PRIORITIES > 0,
		CONFIG_NUM_PREEMPT_PRIORITIES > 0,
		false,
	};
	static int nprio[] = {
		CONFIG_NUM_COOP_PRIORITIES,
		CONFIG_NUM_PREEMPT_PRIORITIES,
		CONFIG_NUM_PREEMPT_PRIORITIES,
		42,
	};
	const char *const prios[] = {"pmin", "pmax"};

	zassert_ok(pthread_attr_init(&attr));

	BUILD_ASSERT(!(SCHED_INVALID == SCHED_FIFO || SCHED_INVALID == SCHED_RR ||
		       SCHED_INVALID == SCHED_OTHER),
		     "SCHED_INVALID is itself invalid");

	ARRAY_FOR_EACH(policies, policy) {
		ARRAY_FOR_EACH(prios, i) {
			errno = 0;
			if (i == 0) {
				pmin = sched_get_priority_min(policies[policy]);
				param.sched_priority = pmin;
			} else {
				pmax = sched_get_priority_max(policies[policy]);
				param.sched_priority = pmax;
			}

			if (policy == 3) {
				zassert_equal(-1, param.sched_priority);
				zassert_equal(errno, EINVAL);
				continue;
			}

			zassert_not_equal(-1, param.sched_priority,
					  "sched_get_priority_%s(%s) failed: %d",
					  i == 0 ? "min" : "max", policy_names[policy], errno);
			zassert_ok(errno, "sched_get_priority_%s(%s) set errno to %d",
				   i == 0 ? "min" : "max", policy_names[policy], errno);
		}

		if (policy != 3) {
			zassert_true(pmax > pmin,
				     "%s min/max inconsistency, pmax (%d) <= pmin (%d)",
				     policy_names[policy], pmax, pmin);
			zassert_equal(pmin, 0, "unexpected pmin for %s", policy_names[policy]);
			zassert_equal(pmax, nprio[policy] - 1, "unexpected pmax for %s",
				      policy_names[policy]);
		}

		ARRAY_FOR_EACH(prios, j) {
			param.sched_priority = (j == 0) ? pmin : pmax;

			if (!policy_enabled[policy]) {
				zassert_not_ok(
					pthread_attr_setschedpolicy(&attr, policies[policy]));
				zassert_not_ok(
					pthread_attr_setschedparam(&attr, &param),
					"pthread_attr_setschedparam() failed for %s (%d) of %s",
					prios[j], param.sched_priority, policy_names[policy]);
				continue;
			}

			zassert_ok(pthread_attr_setschedpolicy(&attr, policies[policy]),
				   "pthread_attr_setschedpolicy() failed for %s (%d) of %s",
				   prios[j], param.sched_priority, policy_names[policy]);
			zassert_ok(pthread_attr_setschedparam(&attr, &param),
				   "pthread_attr_setschedparam() failed for %s (%d) of %s",
				   prios[j], param.sched_priority, policy_names[policy]);

			can_create_thread(&attr);
		}
	}

	zassert_ok(pthread_attr_destroy(&attr));
}

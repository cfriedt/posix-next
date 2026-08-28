/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <limits.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(posix_pthread_priority, CONFIG_PTHREAD_LOG_LEVEL);

#define ZEPHYR_TO_POSIX_PRIORITY(_zprio)                                                           \
	(((_zprio) < 0) ? (-1 * ((_zprio) + 1)) : (CONFIG_NUM_PREEMPT_PRIORITIES - (_zprio)-1))

#define POSIX_TO_ZEPHYR_PRIORITY(_prio, _pol)                                                      \
	(((_pol) == SCHED_FIFO) ? (-1 * ((_prio) + 1))                                             \
				: (CONFIG_NUM_PREEMPT_PRIORITIES - (_prio)-1))

bool is_posix_policy_prio_valid(int priority, int policy)
{
	if ((priority >= posix_sched_priority_min(policy)) &&
	    (priority <= posix_sched_priority_max(policy))) {
		return true;
	}

	LOG_DBG("Invalid priority %d and / or policy %d", priority, policy);

	return false;
}

int zephyr_to_posix_priority(int z_prio, int *policy)
{
	int priority;

	z_prio = CLAMP(z_prio, K_HIGHEST_THREAD_PRIO, CONFIG_NUM_PREEMPT_PRIORITIES - 1);
	*policy = (z_prio < 0) ? SCHED_FIFO : SCHED_RR;
	priority = ZEPHYR_TO_POSIX_PRIORITY(z_prio);
	__ASSERT_NO_MSG(is_posix_policy_prio_valid(priority, *policy));

	return priority;
}

int posix_to_zephyr_priority(int priority, int policy)
{
	__ASSERT_NO_MSG(is_posix_policy_prio_valid(priority, policy));

	return POSIX_TO_ZEPHYR_PRIORITY(priority, policy);
}

int posix_thread_attr_default_priority(void)
{
	int policy = IS_ENABLED(CONFIG_PREEMPT_ENABLED) ? SCHED_RR : SCHED_FIFO;

	return POSIX_TO_ZEPHYR_PRIORITY(K_LOWEST_APPLICATION_THREAD_PRIO, policy);
}

/*
 * Copyright (c) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <time.h>

#include <zephyr/kernel.h>
#include <sched.h>

/*
 * Zephyr does not support processes; the single (implicit) process is
 * designated either by 0 ("the calling process") or by the value that
 * getpid() returns, and its scheduling parameters are those of the calling
 * thread, as on Linux.
 */
static bool this_process(pid_t pid)
{
	if ((pid == 0) || (pid == POSIX_THIS_PID)) {
		return true;
	}

	errno = ESRCH;

	return false;
}

int sched_get_priority_min(int policy)
{
	return posix_sched_priority_min(policy);
}

int sched_get_priority_max(int policy)
{
	return posix_sched_priority_max(policy);
}

int sched_getparam(pid_t pid, struct sched_param *param)
{
	int policy;

	if (!this_process(pid)) {
		return -1;
	}

	if (param == NULL) {
		errno = EINVAL;
		return -1;
	}

	param->sched_priority =
		zephyr_to_posix_priority(k_thread_priority_get(k_current_get()), &policy);

	return 0;
}

int sched_getscheduler(pid_t pid)
{
	int policy;

	if (!this_process(pid)) {
		return -1;
	}

	(void)zephyr_to_posix_priority(k_thread_priority_get(k_current_get()), &policy);

	return policy;
}

int sched_setparam(pid_t pid, const struct sched_param *param)
{
	int policy;

	if (!this_process(pid)) {
		return -1;
	}

	if (param == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* the current scheduling policy is retained */
	(void)zephyr_to_posix_priority(k_thread_priority_get(k_current_get()), &policy);

	if (!is_posix_policy_prio_valid(param->sched_priority, policy)) {
		errno = EINVAL;
		return -1;
	}

	k_thread_priority_set(k_current_get(),
			      posix_to_zephyr_priority(param->sched_priority, policy));

	return 0;
}

int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param)
{
	int prev_policy;

	if (!this_process(pid)) {
		return -1;
	}

	if ((param == NULL) || !valid_posix_policy(policy) ||
	    !is_posix_policy_prio_valid(param->sched_priority, policy)) {
		errno = EINVAL;
		return -1;
	}

	(void)zephyr_to_posix_priority(k_thread_priority_get(k_current_get()), &prev_policy);

	k_thread_priority_set(k_current_get(),
			      posix_to_zephyr_priority(param->sched_priority, policy));

	/* the former scheduling policy is returned on success */
	return prev_policy;
}

int sched_rr_get_interval(pid_t pid, struct timespec *interval)
{
	if (!this_process(pid)) {
		return -1;
	}

	if (interval == NULL) {
		errno = EINVAL;
		return -1;
	}

#ifdef CONFIG_TIMESLICING
	*interval = (struct timespec){
		.tv_sec = CONFIG_TIMESLICE_SIZE / MSEC_PER_SEC,
		.tv_nsec = (CONFIG_TIMESLICE_SIZE % MSEC_PER_SEC) * (long)NSEC_PER_MSEC,
	};
#else
	/* no round-robin quantum: threads run until they block or yield */
	*interval = (struct timespec){0};
#endif

	return 0;
}

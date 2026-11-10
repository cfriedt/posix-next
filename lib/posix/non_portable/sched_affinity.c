/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cpu_mask.h"

#include <errno.h>
#include <sched.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

/* 0 is the calling thread; any other value a thread ID, a process leader's being the process ID */
static k_tid_t affinity_target(pid_t pid)
{
	if (pid == 0) {
		return k_current_get();
	}
#ifdef CONFIG_SYS_PROCESS
	if (pid > 0) {
		return sys_thread_find((int)pid);
	}
#endif /* CONFIG_SYS_PROCESS */

	return NULL;
}

int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask)
{
	k_tid_t thread;

	if (mask == NULL) {
		errno = EFAULT;
		return -1;
	}

	if (cpusetsize < sizeof(*mask)) {
		errno = EINVAL;
		return -1;
	}

	thread = affinity_target(pid);
	if (thread == NULL) {
		errno = ESRCH;
		return -1;
	}

	mask_to_cpuset(thread_cpu_mask_get(thread) & CPU_MASK_ALL, cpusetsize, mask);

	return 0;
}

int sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *mask)
{
	k_tid_t thread;
	uint32_t cpus;
	int ret;

	if (mask == NULL) {
		errno = EFAULT;
		return -1;
	}

	/* CPUs that do not exist are dropped, but one must remain */
	cpus = (cpusetsize < sizeof(*mask)) ? 0 : cpuset_to_mask(mask);
	if (cpus == 0) {
		errno = EINVAL;
		return -1;
	}

	thread = affinity_target(pid);
	if (thread == NULL) {
		errno = ESRCH;
		return -1;
	}

	ret = thread_cpu_mask_set(thread, cpus);
	if (ret != 0) {
		errno = ret;
		return -1;
	}

	return 0;
}

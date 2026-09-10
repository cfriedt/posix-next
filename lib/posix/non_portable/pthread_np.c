/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/timeutil.h>

LOG_MODULE_REGISTER(posix_non_portable, CONFIG_POSIX_NON_PORTABLE_LOG_LEVEL);

#define CPU_MASK_ALL BIT_MASK(CONFIG_MP_MAX_NUM_CPUS)

#ifdef CONFIG_SCHED_CPU_MASK
static int thread_cpu_mask_set(struct k_thread *thread, uint32_t mask)
{
	return -k_thread_cpu_mask_set(thread, mask);
}

static uint32_t thread_cpu_mask_get(struct k_thread *thread)
{
	return k_thread_cpu_mask_get(thread);
}
#else
/* every thread may run on every CPU, which is the only set that can be honored */
static int thread_cpu_mask_set(struct k_thread *thread, uint32_t mask)
{
	ARG_UNUSED(thread);

	return (mask == CPU_MASK_ALL) ? 0 : ENOTSUP;
}

static uint32_t thread_cpu_mask_get(struct k_thread *thread)
{
	ARG_UNUSED(thread);

	return CPU_MASK_ALL;
}
#endif /* CONFIG_SCHED_CPU_MASK */

int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize, cpu_set_t *cpuset)
{
	uint32_t mask;

	if (cpuset == NULL) {
		return EFAULT;
	}

	if (cpusetsize < sizeof(*cpuset)) {
		return EINVAL;
	}

	mask = thread_cpu_mask_get(to_k_thread(&thread)) & CPU_MASK_ALL;

	memset(cpuset, 0, cpusetsize);
	for (int cpu = 0; cpu < CONFIG_MP_MAX_NUM_CPUS; ++cpu) {
		if ((mask & BIT(cpu)) != 0) {
			CPU_SET(cpu, cpuset);
		}
	}

	return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset)
{
	uint32_t mask = 0;

	if (cpuset == NULL) {
		return EFAULT;
	}

	if (cpusetsize < sizeof(*cpuset)) {
		return EINVAL;
	}

	for (int cpu = 0; cpu < CONFIG_MP_MAX_NUM_CPUS; ++cpu) {
		if (CPU_ISSET(cpu, cpuset)) {
			mask |= BIT(cpu);
		}
	}

	/* CPUs that do not exist are dropped, but one must remain */
	if (mask == 0) {
		return EINVAL;
	}

	return thread_cpu_mask_set(to_k_thread(&thread), mask);
}

int pthread_timedjoin_np(pthread_t pthread, void **status, const struct timespec *abstime)
{
	/* as on Linux, a NULL abstime blocks indefinitely (and must not be dereferenced) */
	if ((abstime != NULL) && !timespec_is_valid(abstime)) {
		return EINVAL;
	}

	/* TODO(clock-settime-reactive-waits): the CLOCK_REALTIME offset is baked in here */
	int ret = -k_thread_rejoin(
		to_k_thread(&pthread), status,
		(abstime == NULL) ? K_FOREVER
				  : timespec_abs_to_timeout(SYS_CLOCK_REALTIME, abstime));

	if ((ret == EAGAIN) || (ret == EBUSY)) {
		ret = ETIMEDOUT;
	}

	return ret;
}

int pthread_tryjoin_np(pthread_t pthread, void **status)
{
	return -k_thread_rejoin(to_k_thread(&pthread), status, K_NO_WAIT);
}

int pthread_setname_np(pthread_t thread, const char *name)
{
#ifdef CONFIG_THREAD_NAME
	return k_thread_name_set(to_k_thread(&thread), name);
#else
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
	return 0;
#endif
}

int pthread_getname_np(pthread_t thread, char *name, size_t len)
{
#ifdef CONFIG_THREAD_NAME
	char buf[CONFIG_THREAD_MAX_NAME_LEN + 1];
	struct k_thread *const th = to_k_thread(&thread);

	if (name == NULL) {
		return EFAULT;
	}

	if (len < sizeof(buf)) {
		int ret = k_thread_name_copy(th, buf, sizeof(buf) - 1);

		if (ret < 0) {
			return -ret;
		}

		size_t blen = strnlen(buf, sizeof(buf));

		if (blen > len) {
			return ERANGE;
		}

		strncpy(name, buf, len);

		return 0;
	}

	return k_thread_name_copy(th, name, len - 1);
#else
	ARG_UNUSED(thread);
	ARG_UNUSED(name);
	ARG_UNUSED(len);
	return 0;
#endif
}

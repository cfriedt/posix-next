/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "cpu_mask.h"
#include "posix_internal.h"

#include <pthread.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/timeutil.h>

LOG_MODULE_REGISTER(posix_non_portable, CONFIG_POSIX_NON_PORTABLE_LOG_LEVEL);

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
	mask_to_cpuset(mask, cpusetsize, cpuset);

	return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset)
{
	uint32_t mask;

	if (cpuset == NULL) {
		return EFAULT;
	}

	if (cpusetsize < sizeof(*cpuset)) {
		return EINVAL;
	}

	/* CPUs that do not exist are dropped, but one must remain */
	mask = cpuset_to_mask(cpuset);
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

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_NON_PORTABLE_CPU_MASK_H_
#define ZEPHYR_LIB_POSIX_NON_PORTABLE_CPU_MASK_H_

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/posix/sys/cpuset.h>
#include <zephyr/sys/util.h>

#define CPU_MASK_ALL BIT_MASK(CONFIG_MP_MAX_NUM_CPUS)

#ifdef CONFIG_SCHED_CPU_MASK
static inline int thread_cpu_mask_set(struct k_thread *thread, uint32_t mask)
{
	return -k_thread_cpu_mask_set(thread, mask);
}

static inline uint32_t thread_cpu_mask_get(struct k_thread *thread)
{
	return k_thread_cpu_mask_get(thread);
}
#else
/* every thread may run on every CPU, which is the only set that can be honored */
static inline int thread_cpu_mask_set(struct k_thread *thread, uint32_t mask)
{
	ARG_UNUSED(thread);

	return (mask == CPU_MASK_ALL) ? 0 : ENOTSUP;
}

static inline uint32_t thread_cpu_mask_get(struct k_thread *thread)
{
	ARG_UNUSED(thread);

	return CPU_MASK_ALL;
}
#endif /* CONFIG_SCHED_CPU_MASK */

/* the CPUs of @a set that exist; 0 when it names none */
static inline uint32_t cpuset_to_mask(const cpu_set_t *set)
{
	uint32_t mask = 0;

	for (int cpu = 0; cpu < CONFIG_MP_MAX_NUM_CPUS; ++cpu) {
		if (CPU_ISSET(cpu, set)) {
			mask |= BIT(cpu);
		}
	}

	return mask;
}

/* @a mask as a set of @a size bytes */
static inline void mask_to_cpuset(uint32_t mask, size_t size, cpu_set_t *set)
{
	memset(set, 0, size);
	for (int cpu = 0; cpu < CONFIG_MP_MAX_NUM_CPUS; ++cpu) {
		if ((mask & BIT(cpu)) != 0) {
			CPU_SET(cpu, set);
		}
	}
}

#endif /* ZEPHYR_LIB_POSIX_NON_PORTABLE_CPU_MASK_H_ */

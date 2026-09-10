/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief CPU sets for thread affinity (GNU extension, <sched.h>)
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_CPUSET_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_CPUSET_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_CPU_SET_T_DECLARED) || defined(__cpu_set_t_defined)) || defined(__DOXYGEN__)
/**
 * @brief Number of CPUs a @ref cpu_set_t can name (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
#define CPU_SETSIZE 32

/**
 * @brief Set of CPUs for pthread_getaffinity_np() and pthread_setaffinity_np() (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
typedef struct {
	uint32_t __bits[CPU_SETSIZE / 32]; /**< One bit per CPU index. */
} cpu_set_t;
#define _CPU_SET_T_DECLARED
#define __cpu_set_t_defined
#endif

static inline void __cpu_zero(cpu_set_t *set)
{
	for (size_t i = 0; i < CPU_SETSIZE / 32; ++i) {
		set->__bits[i] = 0;
	}
}

static inline int __cpu_count(const cpu_set_t *set)
{
	int count = 0;

	for (size_t i = 0; i < CPU_SETSIZE / 32; ++i) {
		count += __builtin_popcount(set->__bits[i]);
	}

	return count;
}

static inline int __cpu_equal(const cpu_set_t *a, const cpu_set_t *b)
{
	for (size_t i = 0; i < CPU_SETSIZE / 32; ++i) {
		if (a->__bits[i] != b->__bits[i]) {
			return 0;
		}
	}

	return 1;
}

#define __CPU_WORD(cpu) ((size_t)(cpu) / 32)
#define __CPU_BIT(cpu)  (1U << ((size_t)(cpu) % 32))

/**
 * @brief Remove every CPU from @p set (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
#define CPU_ZERO(set) __cpu_zero(set)

/**
 * @brief Add CPU @p cpu to @p set (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
#define CPU_SET(cpu, set)                                                                          \
	((void)(((size_t)(cpu) < CPU_SETSIZE)                                                      \
			? ((set)->__bits[__CPU_WORD(cpu)] |= __CPU_BIT(cpu)) \
			: 0U))

/**
 * @brief Remove CPU @p cpu from @p set (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
#define CPU_CLR(cpu, set)                                                                          \
	((void)(((size_t)(cpu) < CPU_SETSIZE)                                                      \
			? ((set)->__bits[__CPU_WORD(cpu)] &= ~__CPU_BIT(cpu)) \
			: 0U))

/**
 * @brief Test whether CPU @p cpu is in @p set (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
#define CPU_ISSET(cpu, set)                                                                        \
	(((size_t)(cpu) < CPU_SETSIZE) && (((set)->__bits[__CPU_WORD(cpu)] & __CPU_BIT(cpu)) != 0))

/**
 * @brief Number of CPUs in @p set (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
#define CPU_COUNT(set) __cpu_count(set)

/**
 * @brief Test whether @p a and @p b name the same CPUs (GNU extension).
 * @ingroup posix_option_group_non_portable
 */
#define CPU_EQUAL(a, b) __cpu_equal(a, b)

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_CPUSET_H_ */

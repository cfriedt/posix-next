/*
 * Copyright (c) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX process and thread scheduling API (<sched.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sched.h.html">
 *      POSIX.1-2017 &lt;sched.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SCHED_H_
#define ZEPHYR_INCLUDE_POSIX_SCHED_H_

#include <sys/types.h>
#include <time.h>

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Round-robin or other time-sharing scheduling (default).
 * @ingroup posix_option_priority_scheduling
 *
 * May execute identically to @c SCHED_RR on this implementation.
 */
#define SCHED_OTHER 0

/** @brief First-in first-out (cooperative) scheduling policy. @ingroup posix_option_priority_scheduling */
#define SCHED_FIFO 1

/** @brief Round-robin (preemptive, priority-based) scheduling policy. @ingroup posix_option_priority_scheduling */
#define SCHED_RR 2

#if (defined(CONFIG_MINIMAL_LIBC) || defined(CONFIG_PICOLIBC) || defined(CONFIG_ARMCLANG_STD_LIBC) \
	|| defined(CONFIG_ARCMWDT_LIBC)) && !defined(_SCHED_PARAM_DEFINED) && \
	!defined(__sched_param_defined)
/** @brief Scheduling parameters used with sched_setparam() / pthread_attr_setschedparam(). */
struct sched_param {
	int sched_priority; /**< Scheduling priority. */
};
#define _SCHED_PARAM_DEFINED
#define __sched_param_defined
#endif

/**
 * @brief Yield the processor to another thread of equal or higher priority.
 * @ingroup posix_option_group_threads_base
 *
 * @return 0 on success, or -1 with errno set on failure.
 *
 * @see IEEE 1003.1, sched_yield
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_yield.html
 */
int sched_yield(void);

/**
 * @brief Get the minimum priority value for a scheduling policy.
 * @ingroup posix_option_priority_scheduling
 * @param policy @c SCHED_FIFO, @c SCHED_RR, or @c SCHED_OTHER.
 * @return Minimum priority on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_get_priority_min.html
 */
int sched_get_priority_min(int policy);

/**
 * @brief Get the maximum priority value for a scheduling policy.
 * @ingroup posix_option_priority_scheduling
 * @param policy @c SCHED_FIFO, @c SCHED_RR, or @c SCHED_OTHER.
 * @return Maximum priority on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_get_priority_max.html
 */
int sched_get_priority_max(int policy);

/**
 * @brief Get scheduling parameters for a process.
 * @ingroup posix_option_priority_scheduling
 * @param pid   Process ID (0 = calling process).
 * @param param Output: current scheduling parameters.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_getparam.html
 */
int sched_getparam(pid_t pid, struct sched_param *param);

/**
 * @brief Get the scheduling policy of a process.
 * @ingroup posix_option_priority_scheduling
 * @param pid Process ID (0 = calling process).
 * @return Scheduling policy (@c SCHED_FIFO, @c SCHED_RR, @c SCHED_OTHER),
 *         or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_getscheduler.html
 */
int sched_getscheduler(pid_t pid);

/**
 * @brief Set scheduling parameters for a process.
 * @ingroup posix_option_priority_scheduling
 * @param pid   Process ID (0 = calling process).
 * @param param New scheduling parameters.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_setparam.html
 */
int sched_setparam(pid_t pid, const struct sched_param *param);

/**
 * @brief Set the scheduling policy and parameters of a process.
 * @ingroup posix_option_priority_scheduling
 * @param pid    Process ID (0 = calling process).
 * @param policy @c SCHED_FIFO, @c SCHED_RR, or @c SCHED_OTHER.
 * @param param  New scheduling parameters.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_setscheduler.html
 */
int sched_setscheduler(pid_t pid, int policy, const struct sched_param *param);

/**
 * @brief Get the round-robin time quantum for a process.
 * @ingroup posix_option_priority_scheduling
 * @param pid      Process ID (0 = calling process).
 * @param interval Output: round-robin interval.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_rr_get_interval.html
 */
int sched_rr_get_interval(pid_t pid, struct timespec *interval);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SCHED_H_ */

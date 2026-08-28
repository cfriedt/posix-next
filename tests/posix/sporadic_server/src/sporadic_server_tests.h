/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_POSIX_SPORADIC_SERVER_SRC_SPORADIC_SERVER_TESTS_H_
#define TESTS_POSIX_SPORADIC_SERVER_SRC_SPORADIC_SERVER_TESTS_H_

#include <limits.h>
#include <sched.h>

static inline struct sched_param sporadic_param(int priority)
{
	return (struct sched_param){
		.sched_priority = priority,
		.sched_ss_low_priority = sched_get_priority_min(SCHED_SPORADIC),
		.sched_ss_repl_period = {.tv_sec = 1, .tv_nsec = 0},
		.sched_ss_init_budget = {.tv_sec = 0, .tv_nsec = 100000000},
		.sched_ss_max_repl = 1,
	};
}

#endif /* TESTS_POSIX_SPORADIC_SERVER_SRC_SPORADIC_SERVER_TESTS_H_ */

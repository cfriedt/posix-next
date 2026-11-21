/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <sys/resource.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>

int getrusage(int who, struct rusage *r_usage)
{
	uint64_t self_cycles = 0;
	uint64_t child_cycles = 0;
	uint64_t us;
	int ret;

	if ((r_usage == NULL) || ((who != RUSAGE_SELF) && (who != RUSAGE_CHILDREN))) {
		errno = EINVAL;
		return -1;
	}
	ret = k_process_cpu_stats(NULL, &self_cycles, &child_cycles);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	/* the kernel keeps one CPU time per thread, not split by mode: all of it is user time */
	us = k_cyc_to_us_floor64((who == RUSAGE_SELF) ? self_cycles : child_cycles);
	memset(r_usage, 0, sizeof(*r_usage));
	r_usage->ru_utime.tv_sec = (time_t)(us / USEC_PER_SEC);
	r_usage->ru_utime.tv_usec = (suseconds_t)(us % USEC_PER_SEC);

	return 0;
}

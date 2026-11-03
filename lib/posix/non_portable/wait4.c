/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "multi_process_internal.h"

#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>

pid_t wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage)
{
	int ret;
	int kws = 0;
	pid_t num = -1;
	uint64_t cycles = 0;

	if ((options & ~(WNOHANG | WUNTRACED | K_PROCESS_WCONTINUED)) != 0) {
		errno = EINVAL;
		return -1;
	}

	ret = posix_wait_dispatch(pid, &num, &kws, options, &cycles);
	if (ret == -EAGAIN) {
		/* WNOHANG with no state change */
		return 0;
	}
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	if (stat_loc != NULL) {
		*stat_loc = wstatus_to_posix(kws);
	}
	if (rusage != NULL) {
		uint64_t us = k_cyc_to_us_floor64(cycles);

		/* the kernel keeps one CPU time per thread, not split by mode: all of it is user time */
		*rusage = (struct rusage){
			.ru_utime = {.tv_sec = (time_t)(us / USEC_PER_SEC),
				     .tv_usec = (suseconds_t)(us % USEC_PER_SEC)},
		};
	}

	return num;
}

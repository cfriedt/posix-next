/*
 * Copyright (c) 2024, Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/times.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/time_units.h>
#include <zephyr/sys/util.h>

clock_t times(struct tms *buffer)
{
	int ret;
	clock_t utime; /* user time */
	k_thread_runtime_stats_t stats;

	ret = k_thread_runtime_stats_all_get(&stats);
	if (ret < 0) {
		errno = -ret;
		return (clock_t)-1;
	}

	utime = z_tmcvt(stats.total_cycles, sys_clock_hw_cycles_per_sec(), USEC_PER_SEC,
			IS_ENABLED(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) ? false : true,
			sizeof(clock_t) == sizeof(uint32_t), false, false);

	*buffer = (struct tms){
		.tms_utime = utime,
		.tms_stime = 0,
		.tms_cutime = 0,
		.tms_cstime = 0,
	};

	return utime;
}

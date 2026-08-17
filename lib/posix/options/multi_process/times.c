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

static clock_t cyc_to_clock(uint64_t cycles)
{
	return (clock_t)z_tmcvt(cycles, sys_clock_hw_cycles_per_sec(), USEC_PER_SEC,
				IS_ENABLED(CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME) ? false : true,
				sizeof(clock_t) == sizeof(uint32_t), false, false);
}

clock_t times(struct tms *buffer)
{
	uint64_t self_cycles = 0;
	uint64_t child_cycles = 0;

	(void)k_process_cpu_stats(&self_cycles, &child_cycles);

	/*
	 * Zephyr does not distinguish user from system CPU time, so all of a
	 * process's cycles are reported as user time and system time is zero.
	 */
	*buffer = (struct tms){
		.tms_utime = cyc_to_clock(self_cycles),
		.tms_stime = 0,
		.tms_cutime = cyc_to_clock(child_cycles),
		.tms_cstime = 0,
	};

	return (clock_t)k_uptime_ticks();
}

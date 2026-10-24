/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>
#include <unistd.h>

#include <zephyr/sys/clock.h>

/**
 * @brief Sleep for a specified number of seconds.
 *
 * See IEEE 1003.1
 */
unsigned sleep(unsigned int seconds)
{
	struct timespec req = {.tv_sec = seconds};
	struct timespec rem = {0};

	if (sys_clock_nanosleep(SYS_CLOCK_REALTIME, 0, &req, &rem) == 0) {
		return 0;
	}

	/* interrupted: the seconds left, a part of one counting whole */
	return (unsigned int)rem.tv_sec + (rem.tv_nsec > 0);
}

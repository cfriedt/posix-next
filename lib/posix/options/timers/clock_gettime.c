/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2018 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"

#include <errno.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>

int clock_gettime(clockid_t clock_id, struct timespec *ts)
{
	int ret;

	ret = sys_clock_gettime(sys_clock_from_clockid((int)clock_id), ts);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

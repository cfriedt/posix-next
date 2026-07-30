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

int clock_settime(clockid_t clock_id, const struct timespec *tp)
{
	int ret;

	ret = sys_clock_settime(sys_clock_from_clockid((int)clock_id), tp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

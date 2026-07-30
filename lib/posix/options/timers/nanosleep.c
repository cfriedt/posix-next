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

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
	int ret;

	if (rqtp == NULL) {
		errno = EFAULT;
		return -1;
	}

	ret = sys_clock_nanosleep(SYS_CLOCK_REALTIME, 0, rqtp, rmtp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

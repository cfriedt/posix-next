/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"

#include <errno.h>
#include <time.h>

#include <zephyr/sys/clock.h>

int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp,
		    struct timespec *rmtp)
{
	int ret;

	if (rqtp == NULL) {
		errno = EFAULT;
		return -1;
	}

	ret = sys_clock_nanosleep(sys_clock_from_clockid((int)clock_id), flags, rqtp, rmtp);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

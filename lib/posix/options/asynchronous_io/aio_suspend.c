/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "aio_internal.h"

#include <errno.h>
#include <stddef.h>
#include <time.h>

#include <zephyr/posix/aio.h>
#include <zephyr/sys/aio.h>
#include <zephyr/sys_clock.h>

int aio_suspend(const struct aiocb *const list[], int nent, const struct timespec *timeout)
{
	struct sys_aio *handles[CONFIG_SYS_AIO_WAIT_MAX];
	k_timeout_t to = K_FOREVER;
	size_t n = 0;
	int ret;

	if ((list == NULL) || (nent < 0) || (nent > CONFIG_SYS_AIO_WAIT_MAX)) {
		errno = EINVAL;
		return -1;
	}

	for (int i = 0; i < nent; i++) {
		if ((list[i] == NULL) || (list[i]->z_posix_aio_req == NULL) ||
		    (list[i]->z_posix_aio_req == Z_POSIX_AIO_REQ_FAILED)) {
			continue;
		}
		handles[n] = list[i]->z_posix_aio_req;
		n++;
	}

	if (n == 0U) {
		/* nothing outstanding to wait for */
		return 0;
	}

	if (timeout != NULL) {
		if ((timeout->tv_sec < 0) || (timeout->tv_nsec < 0) ||
		    (timeout->tv_nsec >= NSEC_PER_SEC)) {
			errno = EINVAL;
			return -1;
		}
		to = K_MSEC(timeout->tv_sec * MSEC_PER_SEC + timeout->tv_nsec / NSEC_PER_MSEC);
	}

	ret = sys_aio_wait(handles, n, to);
	if (ret < 0) {
		/* a completed-and-retrieved entry is EINVAL from below, as POSIX permits */
		errno = (ret == -EAGAIN) ? EAGAIN : -ret;
		return -1;
	}

	return 0;
}

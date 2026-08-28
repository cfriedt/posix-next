/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "aio_internal.h"

#include <errno.h>
#include <stddef.h>

#include <zephyr/posix/aio.h>
#include <zephyr/sys/aio.h>

int aio_error(const struct aiocb *aiocbp)
{
	int ret;

	if (aiocbp == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (aiocbp->z_posix_aio_req == Z_POSIX_AIO_REQ_FAILED) {
		return EINVAL;
	}

	ret = sys_aio_error(aiocbp->z_posix_aio_req);
	if (ret < 0) {
		/* stale handle, or no operation outstanding */
		errno = EINVAL;
		return -1;
	}

	return ret;
}

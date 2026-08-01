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

ssize_t aio_return(struct aiocb *aiocbp)
{
	int error = 0;
	ssize_t result = -1;

	if (aiocbp == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (aiocbp->z_posix_aio_req == Z_POSIX_AIO_REQ_FAILED) {
		aiocbp->z_posix_aio_req = NULL;
		errno = EINVAL;
		return -1;
	}

	/*
	 * POSIX leaves retrieving the status of an incomplete or already
	 * retrieved request undefined; report EINVAL.
	 */
	if (sys_aio_release(aiocbp->z_posix_aio_req, &error, &result) < 0) {
		errno = EINVAL;
		return -1;
	}

	aiocbp->z_posix_aio_req = NULL;

	if (result < 0) {
		errno = error;
	}

	return result;
}

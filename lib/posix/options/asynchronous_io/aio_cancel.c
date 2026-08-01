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

int aio_cancel(int fildes, struct aiocb *aiocbp)
{
	int ret;

	if ((aiocbp != NULL) && (aiocbp->z_posix_aio_req == Z_POSIX_AIO_REQ_FAILED)) {
		/* already complete (with EINVAL); nothing left to cancel */
		return AIO_ALLDONE;
	}

	ret = sys_aio_cancel(fildes, (aiocbp == NULL) ? NULL : aiocbp->z_posix_aio_req);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	switch (ret) {
	case SYS_AIO_CANCELED:
		return AIO_CANCELED;
	case SYS_AIO_NOTCANCELED:
		return AIO_NOTCANCELED;
	default:
		return AIO_ALLDONE;
	}
}

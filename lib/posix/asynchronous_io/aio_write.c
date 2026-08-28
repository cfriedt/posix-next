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

int aio_write(struct aiocb *aiocbp)
{
	if (aiocbp == NULL) {
		errno = EINVAL;
		return -1;
	}

	return z_posix_aio_submit(aiocbp, SYS_AIO_OP_WRITE, NULL);
}

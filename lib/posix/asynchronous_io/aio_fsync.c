/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "aio_internal.h"

#include <errno.h>
#include <stddef.h>

#include <zephyr/posix/aio.h>
#include <zephyr/posix/fcntl.h>
#include <zephyr/sys/aio.h>

int aio_fsync(int op, struct aiocb *aiocbp)
{
	/* zvfs synchronizes data and metadata together: O_DSYNC == O_SYNC */
	if ((aiocbp == NULL) || ((op != O_SYNC) && (op != O_DSYNC))) {
		errno = EINVAL;
		return -1;
	}

	return z_posix_aio_submit(aiocbp, SYS_AIO_OP_FSYNC, NULL);
}

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
#include <zephyr/toolchain.h>

int lio_listio(int mode, struct aiocb *const list[ZRESTRICT], int nent,
	       struct sigevent *ZRESTRICT sig)
{
	struct sys_aio_group *grp = NULL;
	bool failed = false;
	bool rejected = false;
	int ret;

	if (((mode != LIO_WAIT) && (mode != LIO_NOWAIT)) || (list == NULL) || (nent < 0) ||
	    (nent > CONFIG_POSIX_AIO_LISTIO_MAX)) {
		errno = EINVAL;
		return -1;
	}

	if ((mode == LIO_NOWAIT) && (sig != NULL) && (sig->sigev_notify != SIGEV_NONE)) {
		struct sys_aio_notify notify = {0};

		if (z_posix_aio_notify_from_sigevent(&notify, sig) < 0) {
			return -1;
		}
		ret = sys_aio_group_create(&notify, &grp);
		if (ret < 0) {
			errno = (ret == -EAGAIN) ? EAGAIN : -ret;
			return -1;
		}
	}

	for (int i = 0; i < nent; i++) {
		struct aiocb *acb = list[i];

		if ((acb == NULL) || (acb->aio_lio_opcode == LIO_NOP)) {
			continue;
		}

		switch (acb->aio_lio_opcode) {
		case LIO_READ:
			ret = z_posix_aio_submit(acb, SYS_AIO_OP_READ, grp);
			break;
		case LIO_WRITE:
			ret = z_posix_aio_submit(acb, SYS_AIO_OP_WRITE, grp);
			break;
		default:
			/* as on Linux, an unknown opcode fails the entry, not the list */
			acb->z_posix_aio_req = Z_POSIX_AIO_REQ_FAILED;
			failed = true;
			continue;
		}

		if (ret < 0) {
			failed = true;
			rejected = true;
		}
	}

	if (grp != NULL) {
		/* arm the list notification: fires when the last member completes */
		(void)sys_aio_group_finalize(grp);
	}

	if (mode == LIO_NOWAIT) {
		if (rejected) {
			errno = EIO;
			return -1;
		}
		return 0;
	}

	/* LIO_WAIT: await every submitted entry; the caller reaps with aio_return() */
	for (int i = 0; i < nent; i++) {
		struct aiocb *acb = list[i];
		struct sys_aio *handle;

		if ((acb == NULL) || (acb->aio_lio_opcode == LIO_NOP) ||
		    (acb->z_posix_aio_req == NULL) ||
		    (acb->z_posix_aio_req == Z_POSIX_AIO_REQ_FAILED)) {
			continue;
		}

		handle = acb->z_posix_aio_req;
		(void)sys_aio_wait(&handle, 1, K_FOREVER);
		if (sys_aio_error(handle) != 0) {
			failed = true;
		}
	}

	if (failed) {
		errno = EIO;
		return -1;
	}

	return 0;
}

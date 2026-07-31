/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_MESSAGE_PASSING_MQUEUE_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_MESSAGE_PASSING_MQUEUE_INTERNAL_H_

#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/types.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/mqueue.h>
#include <zephyr/toolchain.h>

/*
 * mqd_t is a zvfs file descriptor and the queue itself is a kernel object from
 * the sys_msgq pool: this layer only translates POSIX spellings into
 * sys_msgq_*() calls. There is no module-side per-queue state.
 */

static ALWAYS_INLINE int to_oflags(int oflags)
{
	int zflags = 0;

	switch (oflags & O_ACCMODE) {
	case O_WRONLY:
		zflags = ZVFS_O_WRONLY;
		break;
	case O_RDWR:
		zflags = ZVFS_O_RDWR;
		break;
	default:
		zflags = ZVFS_O_RDONLY;
		break;
	}

	if ((oflags & O_CREAT) != 0) {
		zflags |= ZVFS_O_CREAT;
	}
	if ((oflags & O_EXCL) != 0) {
		zflags |= ZVFS_O_EXCL;
	}
	if ((oflags & O_NONBLOCK) != 0) {
		zflags |= ZVFS_O_NONBLOCK;
	}

	return zflags;
}

/*
 * Translate a transfer failure to errno. The kernel distinguishes "did not
 * wait" (-ENOMSG) from "waited and the deadline passed" (-EAGAIN), while POSIX
 * distinguishes a non-blocking descriptor (EAGAIN) from an expired timeout
 * (ETIMEDOUT). The two disagree in one case: a timed call whose deadline has
 * already passed does not wait either, so the descriptor's flags decide.
 */
static inline int mq_transfer_errno(mqd_t mqdes, int ret, bool timed)
{
	int oflags = 0;

	if (!timed) {
		return (ret == -ENOMSG) ? EAGAIN : -ret;
	}

	if (ret == -EAGAIN) {
		return ETIMEDOUT;
	}
	if (ret != -ENOMSG) {
		return -ret;
	}

	if ((sys_msgq_getattr((int)mqdes, NULL, &oflags) == 0) &&
	    ((oflags & ZVFS_O_NONBLOCK) != 0)) {
		return EAGAIN;
	}

	return ETIMEDOUT;
}

static inline int mq_send_common(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
				 unsigned int msg_prio, k_timeout_t timeout, bool timed)
{
	int ret;

	if (msg_prio >= CONFIG_POSIX_MQ_PRIO_MAX) {
		errno = EINVAL;
		return -1;
	}

	ret = sys_msgq_send((int)mqdes, msg_ptr, msg_len, msg_prio, timeout);
	if (ret < 0) {
		errno = mq_transfer_errno(mqdes, ret, timed);
		return -1;
	}

	return 0;
}

static inline ssize_t mq_receive_common(mqd_t mqdes, char *msg_ptr, size_t msg_len,
					unsigned int *msg_prio, k_timeout_t timeout, bool timed)
{
	int ret;
	uint32_t prio;

	ret = sys_msgq_receive((int)mqdes, msg_ptr, msg_len, &prio, timeout);
	if (ret < 0) {
		errno = mq_transfer_errno(mqdes, ret, timed);
		return -1;
	}

	if (msg_prio != NULL) {
		*msg_prio = prio;
	}

	return ret;
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_MESSAGE_PASSING_MQUEUE_INTERNAL_H_ */

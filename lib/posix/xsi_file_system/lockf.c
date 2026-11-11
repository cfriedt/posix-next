/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>

int lockf(int fildes, int function, off_t size)
{
	struct zvfs_flock lock = {.l_type = ZVFS_F_WRLCK, .l_whence = ZVFS_SEEK_CUR};
	int cmd;

	if (size < 0) {
		/* the region ends at the current offset */
		off_t pos = zvfs_lseek(fildes, 0, ZVFS_SEEK_CUR);

		if (pos < 0) {
			return -1;
		}
		if (pos + size < 0) {
			errno = EINVAL;
			return -1;
		}
		lock.l_whence = ZVFS_SEEK_SET;
		lock.l_start = (uint64_t)(pos + size);
		lock.l_len = (uint64_t)-size;
	} else {
		lock.l_len = (uint64_t)size;
	}

	switch (function) {
	case F_ULOCK:
		lock.l_type = ZVFS_F_UNLCK;
		cmd = ZVFS_F_SETLK;
		break;
	case F_LOCK:
		cmd = ZVFS_F_SETLKW;
		break;
	case F_TLOCK:
		cmd = ZVFS_F_SETLK;
		break;
	case F_TEST:
		if (zvfs_fcntl(fildes, ZVFS_F_GETLK, (uintptr_t)&lock) < 0) {
			return -1;
		}
		if (lock.l_type == ZVFS_F_UNLCK) {
			return 0;
		}
		errno = EAGAIN;
		return -1;
	default:
		errno = EINVAL;
		return -1;
	}

	return zvfs_fcntl(fildes, cmd, (uintptr_t)&lock);
}

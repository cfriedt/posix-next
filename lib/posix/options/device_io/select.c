/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <zephyr/sys/clock.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>

typedef struct zvfs_fd_set fd_set;

#undef FD_CLR
void FD_CLR(int fd, fd_set *fdset)
{
	ZVFS_FD_CLR(fd, (struct zvfs_fd_set *)fdset);
}

#undef FD_ISSET
int FD_ISSET(int fd, fd_set *fdset)
{
	return ZVFS_FD_ISSET(fd, (struct zvfs_fd_set *)fdset);
}

#undef FD_SET
void FD_SET(int fd, fd_set *fdset)
{
	ZVFS_FD_SET(fd, (struct zvfs_fd_set *)fdset);
}

#undef FD_ZERO
void FD_ZERO(fd_set *fdset)
{
	ZVFS_FD_ZERO((struct zvfs_fd_set *)fdset);
}

struct timeval;
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	struct timespec to;

	if (timeout != NULL) {
		if (!timeval_to_timespec(timeout, &to)) {
			errno = EINVAL;
			return -1;
		}
	}

	/* FIXME: zvfs_select() along with many zvfs system calls, should return a negative error
	 * code on failure instead of -1 and setting errno. Setting errno should be done by the
	 * caller.
	 */
	return zvfs_select(nfds, (struct zvfs_fd_set *)readfds, (struct zvfs_fd_set *)writefds,
			   (struct zvfs_fd_set *)exceptfds, &to, NULL);
}

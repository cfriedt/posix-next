/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <signal.h>
#include <sys/select.h>
#include <time.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>

int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	    const struct timespec *timeout, const sigset_t *sigmask)
{
	return zvfs_select(nfds, (struct zvfs_fd_set *)readfds, (struct zvfs_fd_set *)writefds,
			   (struct zvfs_fd_set *)exceptfds, timeout, sigmask);
}

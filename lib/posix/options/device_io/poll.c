/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <poll.h>

#include <zephyr/sys/zvfs.h>

int poll(struct pollfd *fds, int nfds, int timeout)
{
	return zvfs_poll(fds, nfds, timeout);
}

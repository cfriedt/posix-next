/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/socket.h>

#include <zephyr/net/socket.h>

#include "zsock_conversion.h"

ssize_t sendmsg(int sock, const struct msghdr *message, int flags)
{
	struct posix_zsock_msghdr_buf zbuf;

	if (message == NULL) {
		errno = EFAULT;
		return -1;
	}

	return zsock_sendmsg(sock, posix_msghdr_to_zsock(message, &zbuf), flags);
}

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/socket.h>

#include <zephyr/net/socket.h>

#include "zsock_conversion.h"

ssize_t recvmsg(int sock, struct msghdr *msg, int flags)
{
	struct posix_zsock_msghdr_buf zbuf;
	struct zsock_msghdr *zmsg;
	ssize_t ret;

	if (msg == NULL) {
		return zsock_recvmsg(sock, NULL, flags);
	}

	zmsg = posix_msghdr_for_recv(msg, &zbuf);
	ret = zsock_recvmsg(sock, zmsg, flags);
	if (ret < 0) {
		return ret;
	}

	zsock_msghdr_to_posix(zmsg, msg, &zbuf);
	return ret;
}

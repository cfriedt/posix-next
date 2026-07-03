/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/socket.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include "zsock_conversion.h"

ssize_t sendmsg(int sock, const struct msghdr *message, int flags)
{
	struct posix_zsock_sockaddr_buf zaddr;
	struct zsock_msghdr zmsg;
	size_t zaddrlen = sizeof(zaddr);

	if (message == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (message->msg_name == NULL || message->msg_namelen == 0) {
		zmsg.msg_name = NULL;
		zmsg.msg_namelen = 0;
	} else {
		zmsg.msg_name = posix_sockaddr_to_zsock(message->msg_name, message->msg_namelen,
							posix_zsock_sa_buf(&zaddr), &zaddrlen);
		zmsg.msg_namelen = (socklen_t)zaddrlen;
	}

	zmsg.msg_iov = (struct zsock_iovec *)message->msg_iov;
	zmsg.msg_iovlen = message->msg_iovlen;
	zmsg.msg_control = message->msg_control;
	zmsg.msg_controllen = message->msg_controllen;
	zmsg.msg_flags = message->msg_flags;

	return zsock_sendmsg(sock, &zmsg, flags);
}

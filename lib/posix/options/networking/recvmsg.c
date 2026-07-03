/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/socket.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include "zsock_conversion.h"

ssize_t recvmsg(int sock, struct msghdr *msg, int flags)
{
	struct posix_zsock_sockaddr_buf zaddr;
	struct zsock_msghdr zmsg;
	size_t out_len;
	ssize_t ret;

	if (msg == NULL) {
		errno = EINVAL;
		return -1;
	}

	zmsg.msg_iov = (struct zsock_iovec *)msg->msg_iov;
	zmsg.msg_iovlen = msg->msg_iovlen;
	zmsg.msg_control = msg->msg_control;
	zmsg.msg_controllen = msg->msg_controllen;
	zmsg.msg_flags = msg->msg_flags;

	if (msg->msg_name != NULL && msg->msg_namelen > 0) {
		zmsg.msg_name = posix_zsock_sa_buf(&zaddr);
		zmsg.msg_namelen = sizeof(zaddr);
	} else {
		zmsg.msg_name = NULL;
		zmsg.msg_namelen = 0;
	}

	ret = zsock_recvmsg(sock, &zmsg, flags);
	if (ret < 0) {
		return ret;
	}

	if (msg->msg_name != NULL && msg->msg_namelen > 0 && zmsg.msg_namelen > 0) {
		out_len = msg->msg_namelen;
		zsock_sockaddr_to_posix(posix_zsock_sa_buf(&zaddr), zmsg.msg_namelen, msg->msg_name, &out_len);
		msg->msg_namelen = (socklen_t)out_len;
	}

	msg->msg_flags = zmsg.msg_flags;
	return ret;
}

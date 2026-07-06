/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include "zsock_conversion.h"

int accept(int sock, struct sockaddr *addr, socklen_t *addrlen)
{
	struct posix_zsock_sockaddr_buf zbuf;
	socklen_t zaddrlen = sizeof(zbuf);
	size_t out_len;
	int ret;

	if (addr == NULL || addrlen == NULL) {
		return zsock_accept(sock, NULL, addrlen);
	}

	ret = zsock_accept(sock, posix_zsock_sa_buf(&zbuf), &zaddrlen);
	if (ret < 0) {
		return ret;
	}

	out_len = *addrlen;
	zsock_sockaddr_to_posix(posix_zsock_sa_buf(&zbuf), zaddrlen, addr, &out_len);
	*addrlen = (socklen_t)out_len;
	return ret;
}


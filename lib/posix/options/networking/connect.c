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

int connect(int sock, const struct sockaddr *addr, socklen_t addrlen)
{
	struct posix_zsock_sockaddr_buf zbuf;
	size_t zaddrlen = sizeof(zbuf);

	if (addr == NULL) {
		errno = EDESTADDRREQ;
		return -1;
	}

	return zsock_connect(
		sock, posix_sockaddr_to_zsock(addr, addrlen, posix_zsock_sa_buf(&zbuf), &zaddrlen),
		(socklen_t)zaddrlen);
}

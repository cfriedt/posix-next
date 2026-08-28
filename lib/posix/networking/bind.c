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

#include <zephyr/posix/net/conversion.h>

int bind(int sock, const struct sockaddr *addr, socklen_t addrlen)
{
	struct sockaddr_storage zbuf;
	size_t zaddrlen = sizeof(zbuf);

	if (addr == NULL) {
		errno = EDESTADDRREQ;
		return -1;
	}

	struct net_sockaddr *zaddr =
		posix_sockaddr_to_zephyr(addr, addrlen, (struct net_sockaddr *)&zbuf, &zaddrlen);

	return zsock_bind(sock, zaddr, (socklen_t)zaddrlen);
}

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

ssize_t sendto(int sock, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
	       socklen_t addrlen)
{
	struct sockaddr_storage zbuf;
	size_t zaddrlen = sizeof(zbuf);

	if (dest_addr == NULL) {
		return zsock_sendto(sock, buf, len, flags, NULL, 0);
	}

	struct net_sockaddr *zaddr =
		posix_sockaddr_to_zephyr(dest_addr, addrlen, (struct net_sockaddr *)&zbuf, &zaddrlen);

	return zsock_sendto(sock, buf, len, flags, zaddr, (socklen_t)zaddrlen);
}


/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include <zephyr/posix/net/conversion.h>

int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
		char *serv, socklen_t servlen, int flags)
{
	struct sockaddr_storage zbuf;
	size_t zaddrlen = sizeof(zbuf);

	if (addr == NULL) {
		return zsock_getnameinfo(NULL, 0, host, hostlen, serv, servlen, flags);
	}

	struct net_sockaddr *zaddr =
		posix_sockaddr_to_zephyr(addr, addrlen, (struct net_sockaddr *)&zbuf, &zaddrlen);

	return zsock_getnameinfo(zaddr, (socklen_t)zaddrlen, host, hostlen, serv, servlen, flags);
}


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

#include "zsock_conversion.h"

int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
		char *serv, socklen_t servlen, int flags)
{
	struct posix_zsock_sockaddr_buf zbuf;
	size_t zaddrlen = sizeof(zbuf);

	if (addr == NULL) {
		return zsock_getnameinfo(NULL, 0, host, hostlen, serv, servlen, flags);
	}

	return zsock_getnameinfo(posix_sockaddr_to_zsock(addr, addrlen, posix_zsock_sa_buf(&zbuf), &zaddrlen),
				 (socklen_t)zaddrlen, host, hostlen, serv, servlen, flags);
}


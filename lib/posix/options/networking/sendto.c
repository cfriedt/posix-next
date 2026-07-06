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

ssize_t sendto(int sock, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
	       socklen_t addrlen)
{
	struct posix_zsock_sockaddr_buf zbuf;
	size_t zaddrlen = sizeof(zbuf);

	if (dest_addr == NULL) {
		return zsock_sendto(sock, buf, len, flags, NULL, 0);
	}

	return zsock_sendto(sock, buf, len, flags,
			    posix_sockaddr_to_zsock(dest_addr, addrlen, posix_zsock_sa_buf(&zbuf), &zaddrlen),
			    (socklen_t)zaddrlen);
}


/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include "zsock_conversion.h"

ssize_t recvfrom(int sock, void *buf, size_t max_len, int flags, struct sockaddr *src_addr,
		 socklen_t *addrlen)
{
	struct posix_zsock_sockaddr_buf zbuf;
	socklen_t zaddrlen = sizeof(zbuf);
	size_t out_len;
	ssize_t ret;

	if (src_addr == NULL || addrlen == NULL) {
		return zsock_recvfrom(sock, buf, max_len, flags, NULL, NULL);
	}

	ret = zsock_recvfrom(sock, buf, max_len, flags, posix_zsock_sa_buf(&zbuf), &zaddrlen);
	if (ret < 0) {
		return ret;
	}

	out_len = *addrlen;
	zsock_sockaddr_to_posix(posix_zsock_sa_buf(&zbuf), zaddrlen, src_addr, &out_len);
	*addrlen = (socklen_t)out_len;
	return ret;
}

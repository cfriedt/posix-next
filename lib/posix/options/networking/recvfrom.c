/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include <zephyr/posix/net/conversion.h>

ssize_t recvfrom(int sock, void *buf, size_t max_len, int flags, struct sockaddr *src_addr,
		 socklen_t *addrlen)
{
	struct sockaddr_storage zbuf;
	socklen_t zaddrlen = sizeof(zbuf);
	size_t out_len;
	struct sockaddr *res;
	ssize_t ret;

	if (src_addr == NULL || addrlen == NULL) {
		return zsock_recvfrom(sock, buf, max_len, flags, NULL, NULL);
	}

	ret = zsock_recvfrom(sock, buf, max_len, flags, (struct net_sockaddr *)&zbuf, &zaddrlen);
	if (ret < 0) {
		return ret;
	}

	out_len = *addrlen;
	res = zephyr_sockaddr_to_posix((struct net_sockaddr *)&zbuf, zaddrlen, src_addr, &out_len);
	if (res != src_addr) {
		memcpy(src_addr, res, out_len);
	}
	*addrlen = (socklen_t)out_len;
	return ret;
}

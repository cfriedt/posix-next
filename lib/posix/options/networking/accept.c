/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include <zephyr/posix/net/conversion.h>

int accept(int sock, struct sockaddr *addr, socklen_t *addrlen)
{
	struct sockaddr_storage zbuf;
	socklen_t zaddrlen = sizeof(zbuf);
	size_t out_len;
	struct sockaddr *res;
	int ret;

	if (addr == NULL || addrlen == NULL) {
		return zsock_accept(sock, NULL, addrlen);
	}

	ret = zsock_accept(sock, (struct net_sockaddr *)&zbuf, &zaddrlen);
	if (ret < 0) {
		return ret;
	}

	out_len = *addrlen;
	res = zephyr_sockaddr_to_posix((struct net_sockaddr *)&zbuf, zaddrlen, addr, &out_len);
	if (res != addr) {
		memcpy(addr, res, out_len);
	}
	*addrlen = (socklen_t)out_len;
	return ret;
}


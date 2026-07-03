/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/net/socket.h>

int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints,
		struct addrinfo **res)
{
	return zsock_getaddrinfo(host, service, (const struct zsock_addrinfo *)hints,
				 (struct zsock_addrinfo **)res);
}

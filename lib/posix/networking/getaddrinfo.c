/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <unistd.h>

#include <zephyr/net/socket.h>

#include <zephyr/posix/net/conversion.h>

int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints,
		struct addrinfo **res)
{
	struct zsock_addrinfo zhints;

	return zsock_getaddrinfo(host, service, posix_addrinfo_hints_to_zephyr(hints, &zhints),
				 (struct zsock_addrinfo **)res);
}

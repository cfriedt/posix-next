/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arpa/inet.h>
#include <sys/socket.h>

#include <zephyr/net/socket.h>

int inet_pton(sa_family_t family, const char *src, void *dst)
{
	return zsock_inet_pton(family, src, dst);
}

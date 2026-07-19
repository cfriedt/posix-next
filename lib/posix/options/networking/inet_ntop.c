/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arpa/inet.h>
#include <sys/socket.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

char *inet_ntop(sa_family_t family, const void *src, char *dst, size_t size)
{
	return net_addr_ntop(family, src, dst, size);
}

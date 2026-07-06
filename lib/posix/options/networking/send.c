/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/socket.h>

#include <zephyr/net/socket.h>

ssize_t send(int sock, const void *buf, size_t len, int flags)
{
	return zsock_sendto(sock, buf, len, flags, NULL, 0);
}

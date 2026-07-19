/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/socket.h>

#include <zephyr/net/socket.h>

ssize_t recv(int sock, void *buf, size_t max_len, int flags)
{
	return zsock_recvfrom(sock, buf, max_len, flags, NULL, NULL);
}

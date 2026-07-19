/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/socket.h>

#include <zephyr/net/socket.h>

int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen)
{
	return zsock_setsockopt(sock, level, optname, optval, optlen);
}

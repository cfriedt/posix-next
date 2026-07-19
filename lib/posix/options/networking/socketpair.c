/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/socket.h>

#include <zephyr/net/socket.h>

int socketpair(int family, int type, int proto, int sv[2])
{
	return zsock_socketpair(family, type, proto, sv);
}

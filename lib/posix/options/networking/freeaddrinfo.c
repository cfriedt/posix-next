/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/net/socket.h>

void freeaddrinfo(struct addrinfo *ai)
{
	zsock_freeaddrinfo((struct zsock_addrinfo *)ai);
}

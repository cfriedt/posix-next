/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/net/socket.h>

const char *gai_strerror(int errcode)
{
	return zsock_gai_strerror(errcode);
}

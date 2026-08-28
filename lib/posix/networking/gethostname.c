/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <zephyr/net/socket.h>

int gethostname(char *name, size_t namelen)
{
	return zsock_gethostname(name, namelen);
}

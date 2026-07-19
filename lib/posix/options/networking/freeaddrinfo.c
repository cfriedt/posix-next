/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <netdb.h>

void freeaddrinfo(struct addrinfo *ai)
{
	free(ai);
}

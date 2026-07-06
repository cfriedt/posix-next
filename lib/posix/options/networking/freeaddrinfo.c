/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <netdb.h>

#include "zsock_conversion.h"

void freeaddrinfo(struct addrinfo *ai)
{
	posix_addrinfo_list_free(ai);
}

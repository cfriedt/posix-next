/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>

#include <arpa/inet.h>
#include <netinet/in.h>

char *inet_ntoa(struct in_addr in)
{
	static char buf[INET_ADDRSTRLEN];
	unsigned char *bytes = (unsigned char *)&in.s_addr;

	snprintf(buf, sizeof(buf), "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);

	return buf;
}

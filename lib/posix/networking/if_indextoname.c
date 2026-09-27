/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <net/if.h>

#include <zephyr/net/net_if.h>

char *if_indextoname(unsigned int ifindex, char *ifname)
{
	int ret;

	ret = net_if_get_name(net_if_get_by_index(ifindex), ifname, IF_NAMESIZE);
	if (ret < 0) {
		errno = ENXIO;
		return NULL;
	}

	return ifname;
}

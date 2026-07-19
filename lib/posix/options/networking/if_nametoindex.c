/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <net/if.h>

#include <zephyr/net/net_if.h>

unsigned int if_nametoindex(const char *ifname)
{
	int ret;

	ret = net_if_get_by_name(ifname);
	if (ret < 0) {
		return 0;
	}

	return ret;
}

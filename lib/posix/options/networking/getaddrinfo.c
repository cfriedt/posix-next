/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>

#include <zephyr/net/socket.h>

#include "zsock_conversion.h"

int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints,
		struct addrinfo **res)
{
	struct zsock_addrinfo *zres = NULL;
	struct zsock_addrinfo zhints;
	const struct zsock_addrinfo *zhintsp = NULL;
	struct addrinfo *plist;
	int ret;

	if (res == NULL) {
		errno = EINVAL;
		return EAI_SYSTEM;
	}

	if (hints != NULL) {
		zhintsp = posix_addrinfo_hints_to_zsock(hints, &zhints);
	}

	ret = zsock_getaddrinfo(host, service, zhintsp, &zres);
	if (ret != 0) {
		return ret;
	}

	plist = posix_addrinfo_list_from_zsock(zres);
	if (plist == NULL) {
		zsock_freeaddrinfo(zres);
		return EAI_MEMORY;
	}

	*res = plist;

	if (!posix_addrinfo_layout_eq()) {
		zsock_freeaddrinfo(zres);
	}

	return 0;
}

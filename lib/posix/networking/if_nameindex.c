/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>

#include <net/if.h>

#include <zephyr/net/net_if.h>

void if_freenameindex(struct if_nameindex *ptr)
{
	size_t n;

	if (ptr == NULL) {
		return;
	}

	NET_IFACE_COUNT(&n);

	for (size_t i = 0; i < n; ++i) {
		if (ptr[i].if_name != NULL) {
			free(ptr[i].if_name);
		}
	}

	free(ptr);
}

struct if_nameindex *if_nameindex(void)
{
	size_t n;
	char *name;
	struct if_nameindex *ni;

	NET_IFACE_COUNT(&n);
	ni = malloc((n + 1) * sizeof(*ni));
	if (ni == NULL) {
		goto return_err;
	}

	for (size_t i = 0; i < n; ++i) {
		ni[i].if_index = i + 1;

		ni[i].if_name = malloc(IF_NAMESIZE);
		if (ni[i].if_name == NULL) {
			goto return_err;
		}

		name = if_indextoname(i + 1, ni[i].if_name);
		__ASSERT_NO_MSG(name != NULL);
	}

	ni[n].if_index = 0;
	ni[n].if_name = NULL;

	return ni;

return_err:
	if_freenameindex(ni);
	errno = ENOBUFS;

	return NULL;
}

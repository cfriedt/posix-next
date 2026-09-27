/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <sys/mman.h>

#include <zephyr/sys/zvfs.h>

#include "posix_mman.h"

int mprotect(void *addr, size_t len, int prot)
{
	int zprot = posix_prot_to_zvfs(prot);

	if (zprot < 0) {
		errno = EINVAL;
		return -1;
	}

	return zvfs_mprotect(addr, len, zprot);
}

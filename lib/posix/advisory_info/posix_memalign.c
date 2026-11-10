/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>

/* over the allocator Zephyr gives every C library; a libc's own would bring its heap along */
int posix_memalign(void **memptr, size_t alignment, size_t size)
{
	void *p;

	if (alignment < sizeof(void *) || (alignment & (alignment - 1)) != 0) {
		return EINVAL;
	}

	p = aligned_alloc(alignment, size);
	if (p == NULL && size > 0) {
		return ENOMEM;
	}
	*memptr = p;

	return 0;
}

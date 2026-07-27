/*
 * Copyright (c) 2024, Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

#include <sys/mman.h>
#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>

int mlockall(int flags)
{
	if ((flags == 0) || ((flags & ~(MCL_CURRENT | MCL_FUTURE)) != 0)) {
		errno = EINVAL;
		return -1;
	}

	if (IS_ENABLED(CONFIG_DEMAND_PAGING)) {
		/*
		 * Pages can be evicted, and pinning every current and future
		 * mapping is not yet supported by the Demand Paging API.
		 */
		errno = EAGAIN;
		return -1;
	}

	/* without demand paging, all memory is always resident */
	return 0;
}

int munlockall(void)
{
	/*
	 * Nothing is ever locked by mlockall() under demand paging, and
	 * without it all memory is resident regardless; unlocking everything
	 * is trivially successful either way.
	 */
	return 0;
}

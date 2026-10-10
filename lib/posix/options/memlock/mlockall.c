/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>

#include <sys/mman.h>
#include <zephyr/sys/zvfs.h>

static int mcl_to_zvfs(int flags)
{
	int zflags = 0;

	if ((flags & MCL_CURRENT) != 0) {
		zflags |= ZVFS_MLOCK_CURRENT;
	}
	if ((flags & MCL_FUTURE) != 0) {
		zflags |= ZVFS_MLOCK_FUTURE;
	}

	return zflags;
}

int mlockall(int flags)
{
	if ((flags == 0) || ((flags & ~(MCL_CURRENT | MCL_FUTURE)) != 0)) {
		errno = EINVAL;
		return -1;
	}

	return zvfs_mlock(NULL, 0, mcl_to_zvfs(flags));
}

int munlockall(void)
{
	return zvfs_munlock(NULL, 0, mcl_to_zvfs(MCL_CURRENT | MCL_FUTURE));
}

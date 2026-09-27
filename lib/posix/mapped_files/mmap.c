/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <zephyr/sys/zvfs.h>

#include "posix_mman.h"

/* POSIX mapping flags to ZVFS_MAP_*, or -1 for an unknown bit */
static int flags_to_zvfs(int flags)
{
	int zflags = 0;

	if ((flags & ~(MAP_SHARED | MAP_PRIVATE | MAP_ANONYMOUS)) != 0) {
		return -1;
	}
	if ((flags & MAP_SHARED) != 0) {
		zflags |= ZVFS_MAP_SHARED;
	}
	if ((flags & MAP_PRIVATE) != 0) {
		zflags |= ZVFS_MAP_PRIVATE;
	}
	if ((flags & MAP_ANONYMOUS) != 0) {
		zflags |= ZVFS_MAP_ANONYMOUS;
	}

	return zflags;
}

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	void *virt;
	int zprot;
	int zflags;

	/* the placement hint is not honoured and fixed placement is not supported */
	ARG_UNUSED(addr);

	if ((flags & MAP_FIXED) != 0) {
		errno = ENOTSUP;
		return MAP_FAILED;
	}

	zprot = posix_prot_to_zvfs(prot);
	zflags = flags_to_zvfs(flags);
	if ((zprot < 0) || (zflags < 0) || (off < 0)) {
		errno = EINVAL;
		return MAP_FAILED;
	}

	if (zvfs_mmap(len, zprot, zflags, fildes, (size_t)off, &virt) < 0) {
		return MAP_FAILED;
	}

	return virt;
}

int msync(void *addr, size_t length, int flags)
{
	int zflags = 0;

	if ((flags & ~(MS_SYNC | MS_ASYNC | MS_INVALIDATE)) != 0) {
		errno = EINVAL;
		return -1;
	}
	if ((flags & MS_SYNC) != 0) {
		zflags |= ZVFS_MS_SYNC;
	}
	if ((flags & MS_ASYNC) != 0) {
		zflags |= ZVFS_MS_ASYNC;
	}
	if ((flags & MS_INVALIDATE) != 0) {
		zflags |= ZVFS_MS_INVALIDATE;
	}

	return zvfs_msync(addr, length, zflags);
}

int munmap(void *addr, size_t len)
{
	return zvfs_munmap(addr, len);
}

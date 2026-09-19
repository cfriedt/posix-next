/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <sys/mman.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/toolchain.h>

/* no cache acts on access-pattern advice: it is checked and then kept for nothing */

int posix_fadvise(int fd, off_t offset, off_t len, int advice)
{
	struct zvfs_stat st;

	if (zvfs_fstat(fd, &st) < 0) {
		/* the console descriptors have no attributes to report but are not pipes */
		return (errno == EBADF) ? EBADF : 0;
	}
	if ((st.mode & ZVFS_MODE_IFMT) == ZVFS_MODE_IFIFO) {
		return ESPIPE;
	}
	if (offset < 0 || len < 0) {
		return EINVAL;
	}

	switch (advice) {
	case POSIX_FADV_NORMAL:
	case POSIX_FADV_RANDOM:
	case POSIX_FADV_SEQUENTIAL:
	case POSIX_FADV_WILLNEED:
	case POSIX_FADV_DONTNEED:
	case POSIX_FADV_NOREUSE:
		return 0;
	default:
		return EINVAL;
	}
}

int posix_fallocate(int fd, off_t offset, off_t len)
{
	struct zvfs_stat st;
	int flags;

	const off_t off_max = (off_t)(((uintmax_t)1 << (sizeof(off_t) * CHAR_BIT - 1)) - 1);

	if (offset < 0 || len <= 0) {
		return EINVAL;
	}
	if (offset > off_max - len) {
		return EFBIG;
	}

	flags = zvfs_fcntl(fd, ZVFS_F_GETFL, 0);
	if (flags < 0) {
		return EBADF;
	}
	if ((flags & ZVFS_O_RDWR) == ZVFS_O_RDONLY) {
		return EBADF;
	}
	if (zvfs_fstat(fd, &st) < 0) {
		return errno;
	}
	if ((st.mode & ZVFS_MODE_IFMT) == ZVFS_MODE_IFIFO) {
		return ESPIPE;
	}
	if ((st.mode & ZVFS_MODE_IFMT) != ZVFS_MODE_IFREG) {
		return ENODEV;
	}

	/* the file systems allocate as they extend, so a larger file has the space */
	if (st.size >= offset + len) {
		return 0;
	}
	if (zvfs_ftruncate(fd, offset + len) < 0) {
		return errno;
	}

	return 0;
}

int posix_madvise(void *addr, size_t len, int advice)
{
	ARG_UNUSED(addr);
	ARG_UNUSED(len);

	switch (advice) {
	case POSIX_MADV_NORMAL:
	case POSIX_MADV_RANDOM:
	case POSIX_MADV_SEQUENTIAL:
	case POSIX_MADV_WILLNEED:
	case POSIX_MADV_DONTNEED:
		return 0;
	default:
		return EINVAL;
	}
}

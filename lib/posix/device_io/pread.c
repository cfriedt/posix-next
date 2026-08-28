/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#include <zephyr/sys/zvfs.h>

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
	size_t off = (size_t)offset;

	if (offset < 0) {
		errno = EINVAL;
		return -1;
	}

	return zvfs_read_offset(fd, buf, count, &off);
}

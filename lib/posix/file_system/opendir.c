/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>

#include <errno.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

DIR *opendir(const char *dirname)
{
	int fd = zvfs_openat(ZVFS_AT_FDCWD, dirname, ZVFS_O_RDONLY | ZVFS_O_DIRECTORY, 0);
	DIR *d;

	if (fd < 0) {
		return NULL;
	}

	d = zvfs_fdopendir(fd);
	if (d == NULL) {
		int err = errno;

		(void)zvfs_close(fd);
		errno = err;
	}

	return d;
}

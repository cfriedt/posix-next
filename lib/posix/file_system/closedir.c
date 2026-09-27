/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>

#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

int closedir(DIR *dirp)
{
	if (dirp == NULL) {
		errno = EBADF;
		return -1;
	}

	return zvfs_close(dirp->fd);
}

/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>

#include <zephyr/sys/zvfs_fs.h>

void seekdir(DIR *dirp, long loc)
{
	if (dirp != NULL) {
		(void)zvfs_seekdir(dirp->fd, loc);
	}
}

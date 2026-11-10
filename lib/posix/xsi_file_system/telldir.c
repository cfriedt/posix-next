/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>

#include <zephyr/sys/zvfs_fs.h>

long telldir(DIR *dirp)
{
	if (dirp == NULL) {
		errno = EBADF;
		return -1;
	}

	return zvfs_telldir(dirp->fd);
}

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>

#include <zephyr/sys/zvfs_fs.h>

int dirfd(DIR *dirp)
{
	return dirp->fd;
}

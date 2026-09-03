/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>

#include <zephyr/sys/zvfs_fs.h>

DIR *fdopendir(int fd)
{
	return zvfs_fdopendir(fd);
}

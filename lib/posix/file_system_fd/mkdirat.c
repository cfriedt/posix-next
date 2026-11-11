/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int mkdirat(int fd, const char *path, mode_t mode)
{
	return zvfs_mkdirat(fd, path, mode);
}

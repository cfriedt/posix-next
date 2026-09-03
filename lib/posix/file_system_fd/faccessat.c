/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

int faccessat(int fd, const char *path, int amode, int flag)
{
	return zvfs_accessat(fd, path, amode, flag);
}

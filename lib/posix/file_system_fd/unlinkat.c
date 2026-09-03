/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

int unlinkat(int fd, const char *path, int flag)
{
	return zvfs_unlinkat(fd, path, flag);
}

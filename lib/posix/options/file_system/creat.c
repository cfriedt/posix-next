/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>

int creat(const char *path, mode_t mode)
{
	return zvfs_open(path, ZVFS_O_WRONLY | ZVFS_O_CREAT | ZVFS_O_TRUNC, mode);
}

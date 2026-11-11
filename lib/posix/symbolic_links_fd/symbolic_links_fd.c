/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

ssize_t readlinkat(int fd, const char *ZRESTRICT path, char *ZRESTRICT buf, size_t bufsize)
{
	return zvfs_readlinkat(fd, path, buf, bufsize);
}

int symlinkat(const char *path1, int fd, const char *path2)
{
	return zvfs_symlinkat(path1, fd, path2);
}

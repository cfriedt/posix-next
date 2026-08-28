/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#include <zephyr/sys/zvfs.h>

ssize_t read(int fd, void *buf, size_t sz)
{
	return zvfs_read(fd, buf, sz);
}
#ifdef CONFIG_POSIX_DEVICE_IO_ALIAS_READ
FUNC_ALIAS(read, _read, ssize_t);
#endif /* CONFIG_POSIX_DEVICE_IO_ALIAS_READ */

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/sys/zvfs.h>

int ftruncate(int fd, off_t length)
{
	return zvfs_ftruncate(fd, length);
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_FTRUNCATE
FUNC_ALIAS(ftruncate, _ftruncate, int);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_FTRUNCATE */

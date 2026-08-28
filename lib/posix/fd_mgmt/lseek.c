/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/types.h>
#include <unistd.h>

#include <zephyr/sys/zvfs.h>

off_t lseek(int fd, off_t offset, int whence)
{
	return zvfs_lseek(fd, offset, whence);
}
#ifdef CONFIG_POSIX_FD_MGMT_ALIAS_LSEEK
FUNC_ALIAS(lseek, _lseek, off_t);
#endif /* CONFIG_POSIX_FD_MGMT_ALIAS_LSEEK */

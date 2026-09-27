/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>

#include <zephyr/sys/zvfs_fs.h>

void rewinddir(DIR *dirp)
{
	if (dirp == NULL) {
		/* POSIX defines no failure reporting for rewinddir() */
		return;
	}

	(void)zvfs_rewinddir(dirp->fd);
}

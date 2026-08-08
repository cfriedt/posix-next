/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>

#include <zephyr/fs/fs.h>
#include <zephyr/sys/zvfs_fs.h>

/**
 * @brief Close a directory stream.
 *
 * See IEEE 1003.1
 */
int closedir(DIR *dirp)
{
	int rc;
	struct zvfs_fs_desc *ptr = (struct zvfs_fs_desc *)dirp;

	if (dirp == NULL) {
		errno = EBADF;
		return -1;
	}

	rc = fs_closedir(&ptr->dir);

	zvfs_fs_desc_free(ptr);

	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	return 0;
}

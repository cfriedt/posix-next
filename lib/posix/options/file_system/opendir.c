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
 * @brief Open a directory stream.
 *
 * See IEEE 1003.1
 */
DIR *opendir(const char *dirname)
{
	int rc;
	struct zvfs_fs_desc *ptr;

	ptr = zvfs_fs_desc_alloc(true);
	if (ptr == NULL) {
		errno = EMFILE;
		return NULL;
	}

	fs_dir_t_init(&ptr->dir);

	rc = fs_opendir(&ptr->dir, dirname);
	if (rc < 0) {
		zvfs_fs_desc_free(ptr);
		errno = -rc;
		return NULL;
	}

	return (DIR *)ptr;
}

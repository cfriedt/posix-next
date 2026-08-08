/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include <zephyr/fs/fs.h>
#include <zephyr/sys/zvfs_fs.h>

BUILD_ASSERT(PATH_MAX >= MAX_FILE_NAME, "PATH_MAX is less than MAX_FILE_NAME");

static struct fs_dirent fdirent;
static struct dirent pdirent;

/**
 * @brief Read a directory.
 *
 * See IEEE 1003.1
 */
struct dirent *readdir(DIR *dirp)
{
	int rc;
	struct zvfs_fs_desc *ptr = (struct zvfs_fs_desc *)dirp;

	if (dirp == NULL) {
		errno = EBADF;
		return NULL;
	}

	rc = fs_readdir(&ptr->dir, &fdirent);
	if (rc < 0) {
		errno = -rc;
		return NULL;
	}

	if (fdirent.name[0] == 0) {
		/* assume end-of-dir, leave errno untouched */
		return NULL;
	}

	rc = strlen(fdirent.name);
	rc = (rc < MAX_FILE_NAME) ? rc : (MAX_FILE_NAME - 1);
	(void)memcpy(pdirent.d_name, fdirent.name, rc);

	/* Make sure the name is NULL terminated */
	pdirent.d_name[rc] = '\0';
	return &pdirent;
}

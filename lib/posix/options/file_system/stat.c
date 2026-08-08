/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <zephyr/fs/fs.h>

/**
 * @brief Get file status.
 *
 * See IEEE 1003.1
 */
int stat(const char *path, struct stat *buf)
{
	int rc;
	struct fs_statvfs stat_vfs;
	struct fs_dirent stat_file;

	if (buf == NULL) {
		errno = EBADF;
		return -1;
	}

	rc = fs_statvfs(path, &stat_vfs);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	rc = fs_stat(path, &stat_file);
	if (rc < 0) {
		errno = -rc;
		return -1;
	}

	memset(buf, 0, sizeof(struct stat));

	switch (stat_file.type) {
	case FS_DIR_ENTRY_FILE:
#if defined(_XOPEN_SOURCE)
		buf->st_mode = S_IFREG;
#endif
		break;
	case FS_DIR_ENTRY_DIR:
#if defined(_XOPEN_SOURCE)
		buf->st_mode = S_IFDIR;
#endif
		break;
	default:
		errno = EIO;
		return -1;
	}
	buf->st_size = stat_file.size;
#if defined(_XOPEN_SOURCE)
	buf->st_blksize = stat_vfs.f_bsize;
	/*
	 * This is a best effort guess, as this information is not provided
	 * by the fs_stat function.
	 */
	buf->st_blocks = (stat_file.size + stat_vfs.f_bsize - 1) / stat_vfs.f_bsize;
#endif

	return 0;
}

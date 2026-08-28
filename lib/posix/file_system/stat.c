/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int stat(const char *ZRESTRICT path, struct stat *ZRESTRICT buf)
{
	struct zvfs_stat zs;

	if (zvfs_stat(path, &zs) < 0) {
		return -1;
	}

	memset(buf, 0, sizeof(*buf));
	buf->st_mode = zs.mode;
	buf->st_size = zs.size;
	buf->st_nlink = zs.nlink;
#if defined(_XOPEN_SOURCE)
	buf->st_blksize = zs.blksize;
	buf->st_blocks = zs.blocks;
#endif
	buf->st_atim = zs.atime;
	buf->st_mtim = zs.mtime;
	buf->st_ctim = zs.ctime;

	return 0;
}

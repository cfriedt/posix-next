/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <sys/stat.h>

#include <zephyr/sys/zvfs_fs.h>

int fstatat(int fd, const char *ZRESTRICT path, struct stat *ZRESTRICT buf, int flag)
{
	struct zvfs_stat zs;

	if (zvfs_statat(fd, path, &zs, flag) < 0) {
		return -1;
	}

	memset(buf, 0, sizeof(*buf));
	buf->st_dev = zs.dev;
	buf->st_ino = zs.ino;
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

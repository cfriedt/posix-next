/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_SHARED_POSIX_STAT_H_
#define ZEPHYR_LIB_POSIX_SHARED_POSIX_STAT_H_

#include <string.h>
#include <sys/stat.h>

#include <zephyr/sys/zvfs.h>

/* the POSIX form of a ZVFS status record */
static inline void posix_stat_from_zvfs(struct stat *buf, const struct zvfs_stat *zs)
{
	memset(buf, 0, sizeof(*buf));
	buf->st_dev = zs->dev;
	buf->st_ino = zs->ino;
	buf->st_mode = zs->mode;
	buf->st_nlink = zs->nlink;
	buf->st_uid = zs->uid;
	buf->st_gid = zs->gid;
	buf->st_size = zs->size;
	buf->st_blksize = zs->blksize;
	buf->st_blocks = zs->blocks;
	buf->st_atim = zs->atime;
	buf->st_mtim = zs->mtime;
	buf->st_ctim = zs->ctime;
}

#endif /* ZEPHYR_LIB_POSIX_SHARED_POSIX_STAT_H_ */

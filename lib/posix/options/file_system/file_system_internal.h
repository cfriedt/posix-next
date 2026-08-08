/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_FILE_SYSTEM_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_FILE_SYSTEM_INTERNAL_H_

#include <sys/statvfs.h>

#include <zephyr/sys/zvfs_fs.h>

static inline void statvfs_from_zvfs(struct statvfs *buf, const struct zvfs_statvfs *zv)
{
	buf->f_bsize = zv->f_bsize;
	buf->f_frsize = zv->f_frsize;
	buf->f_blocks = zv->f_blocks;
	buf->f_bfree = zv->f_bfree;
	buf->f_bavail = zv->f_bavail;
	buf->f_files = zv->f_files;
	buf->f_ffree = zv->f_ffree;
	buf->f_favail = zv->f_favail;
	buf->f_fsid = zv->f_fsid;
	buf->f_flag = (zv->f_flag & ZVFS_ST_RDONLY ? ST_RDONLY : 0) |
		      (zv->f_flag & ZVFS_ST_NOSUID ? ST_NOSUID : 0);
	buf->f_namemax = zv->f_namemax;
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_FILE_SYSTEM_INTERNAL_H_ */

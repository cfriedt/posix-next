/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>

int lchown(const char *path, uid_t owner, gid_t group)
{
	/* -1 leaves an ID unchanged, as UINT32_MAX does below */
	uint32_t uid = (owner == (uid_t)-1) ? UINT32_MAX : (uint32_t)owner;
	uint32_t gid = (group == (gid_t)-1) ? UINT32_MAX : (uint32_t)group;

	return zvfs_chownat(ZVFS_AT_FDCWD, path, uid, gid, ZVFS_AT_SYMLINK_NOFOLLOW);
}

int lstat(const char *ZRESTRICT path, struct stat *ZRESTRICT buf)
{
	struct zvfs_stat zs;

	if (zvfs_lstat(path, &zs) < 0) {
		return -1;
	}

	memset(buf, 0, sizeof(*buf));
	buf->st_dev = zs.dev;
	buf->st_ino = zs.ino;
	buf->st_mode = zs.mode;
	buf->st_size = zs.size;
	buf->st_nlink = zs.nlink;
	buf->st_uid = zs.uid;
	buf->st_gid = zs.gid;
#if defined(_XOPEN_SOURCE)
	buf->st_blksize = zs.blksize;
	buf->st_blocks = zs.blocks;
#endif
	buf->st_atim = zs.atime;
	buf->st_mtim = zs.mtime;
	buf->st_ctim = zs.ctime;

	return 0;
}

ssize_t readlink(const char *ZRESTRICT path, char *ZRESTRICT buf, size_t bufsize)
{
	return zvfs_readlink(path, buf, bufsize);
}

int symlink(const char *path1, const char *path2)
{
	return zvfs_symlink(path1, path2);
}

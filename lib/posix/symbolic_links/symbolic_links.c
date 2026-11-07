/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_stat.h"

#include <errno.h>
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

	posix_stat_from_zvfs(buf, &zs);

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

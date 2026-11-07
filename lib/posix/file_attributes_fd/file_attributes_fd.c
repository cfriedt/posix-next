/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>

#include "posix_mode.h"

static int attr_flags(int flag)
{
	if ((flag & ~AT_SYMLINK_NOFOLLOW) != 0) {
		errno = EINVAL;
		return -1;
	}

	return ((flag & AT_SYMLINK_NOFOLLOW) != 0) ? ZVFS_AT_SYMLINK_NOFOLLOW : 0;
}

/* -1 leaves an ID unchanged, as UINT32_MAX does below */
static uint32_t attr_uid(uid_t id)
{
	return (id == (uid_t)-1) ? UINT32_MAX : (uint32_t)id;
}

static uint32_t attr_gid(gid_t id)
{
	return (id == (gid_t)-1) ? UINT32_MAX : (uint32_t)id;
}

int fchmodat(int fd, const char *path, mode_t mode, int flag)
{
	int zflags = attr_flags(flag);

	if (zflags < 0) {
		return -1;
	}

	return zvfs_chmodat(fd, path, posix_mode_to_zvfs(mode & ~S_IFMT), zflags);
}

int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag)
{
	int zflags = attr_flags(flag);

	if (zflags < 0) {
		return -1;
	}

	return zvfs_chownat(fd, path, attr_uid(owner), attr_gid(group), zflags);
}

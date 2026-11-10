/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

#include "posix_mode.h"

/* -1 leaves an ID unchanged, as UINT32_MAX does below */
static uint32_t attr_uid(uid_t id)
{
	return (id == (uid_t)-1) ? UINT32_MAX : (uint32_t)id;
}

static uint32_t attr_gid(gid_t id)
{
	return (id == (gid_t)-1) ? UINT32_MAX : (uint32_t)id;
}

int chmod(const char *path, mode_t mode)
{
	return zvfs_chmod(path, posix_mode_to_zvfs(mode & ~S_IFMT));
}

int fchmod(int fildes, mode_t mode)
{
	return zvfs_fchmod(fildes, posix_mode_to_zvfs(mode & ~S_IFMT));
}

int chown(const char *path, uid_t owner, gid_t group)
{
	return zvfs_chown(path, attr_uid(owner), attr_gid(group));
}

int fchown(int fildes, uid_t owner, gid_t group)
{
	return zvfs_fchown(fildes, attr_uid(owner), attr_gid(group));
}

mode_t umask(mode_t cmask)
{
	return (mode_t)zvfs_umask(posix_mode_to_zvfs(cmask & 0777));
}

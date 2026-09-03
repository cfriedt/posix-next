/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/toolchain.h>

static int attr_at_exists(int fd, const char *path, int flag)
{
	struct zvfs_stat zs;

	return zvfs_statat(fd, path, &zs, flag);
}

int fchmodat(int fd, const char *path, mode_t mode, int flag)
{
	ARG_UNUSED(mode);

	return attr_at_exists(fd, path, flag);
}

int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag)
{
	if (attr_at_exists(fd, path, flag) < 0) {
		return -1;
	}

	/* only the privileged identity exists; -1 leaves an ID unchanged */
	if (((owner != 0) && (owner != (uid_t)-1)) || ((group != 0) && (group != (gid_t)-1))) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

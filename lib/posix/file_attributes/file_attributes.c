/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <zephyr/sys/libc-hooks.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/process_state.h>
#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/toolchain.h>

/*
 * Zephyr file systems store neither owners nor permission bits: every file
 * belongs to the single privileged user and mode changes on an existing file
 * succeed without effect. The file mode creation mask is kept so that
 * umask() round-trips, but with no stored modes it masks nothing.
 */

static Z_LIBC_DATA mode_t attr_cmask;

#if PROCESS_STATE_SUPPORTED
/* an image process keeps its mask with its C library state; the boot image in the static */
struct posix_process_attr {
	mode_t cmask;
};

static mode_t *attr_cmask_ptr(void)
{
	struct process_state *lp = process_state_get();
	struct posix_process_attr *pa;

	if (lp == NULL) {
		return &attr_cmask;
	}
	pa = lp->slot[PROCESS_STATE_SLOT_POSIX];
	if (pa == NULL) {
		pa = calloc(1, sizeof(*pa));
		if (pa == NULL) {
			return &attr_cmask;
		}
		lp->slot[PROCESS_STATE_SLOT_POSIX] = pa;
	}

	return &pa->cmask;
}
#else
static inline mode_t *attr_cmask_ptr(void)
{
	return &attr_cmask;
}
#endif /* PROCESS_STATE_SUPPORTED */

static int attr_path_exists(const char *path)
{
	struct zvfs_stat zs;

	return zvfs_stat(path, &zs);
}

static int attr_fd_exists(int fildes)
{
	struct zvfs_stat zs;

	return zvfs_fstat(fildes, &zs);
}

static int attr_id_valid(uid_t owner, gid_t group)
{
	/* only the privileged identity exists; -1 leaves an ID unchanged */
	if (((owner != 0) && (owner != (uid_t)-1)) || ((group != 0) && (group != (gid_t)-1))) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int chmod(const char *path, mode_t mode)
{
	ARG_UNUSED(mode);

	return attr_path_exists(path);
}

int fchmod(int fildes, mode_t mode)
{
	ARG_UNUSED(mode);

	return attr_fd_exists(fildes);
}

int chown(const char *path, uid_t owner, gid_t group)
{
	if (attr_path_exists(path) < 0) {
		return -1;
	}

	return attr_id_valid(owner, group);
}

int fchown(int fildes, uid_t owner, gid_t group)
{
	if (attr_fd_exists(fildes) < 0) {
		return -1;
	}

	return attr_id_valid(owner, group);
}

mode_t umask(mode_t cmask)
{
	mode_t *mask = attr_cmask_ptr();
	mode_t prev = *mask;

	*mask = cmask & 0777;
	return prev;
}

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_FILE_SYSTEM_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_FILE_SYSTEM_INTERNAL_H_

#include <errno.h>
#include <limits.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <zephyr/sys/util_macro.h>
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

/*
 * Shared by pathconf() and fpathconf(). Switches on the _PC_* names, never
 * ordinals: newlib and picolibc number them differently. A variable with no
 * association returns -1 with errno unchanged; only an unrecognized name
 * sets EINVAL.
 */
static inline long posix_pathconf_value(int name, const struct zvfs_statvfs *zv)
{
	switch (name) {
	case _PC_2_SYMLINKS:
		return 0;
	case _PC_ALLOC_SIZE_MIN:
	case _PC_REC_INCR_XFER_SIZE:
	case _PC_REC_MIN_XFER_SIZE:
	case _PC_REC_XFER_ALIGN:
		/* the fundamental block size, when the mount reports one */
		return (zv != NULL && zv->f_frsize > 0) ? (long)zv->f_frsize : -1;
	case _PC_REC_MAX_XFER_SIZE:
		return -1;
	case _PC_ASYNC_IO:
		return IS_ENABLED(CONFIG_POSIX_ASYNCHRONOUS_IO) ? 1 : -1;
	case _PC_CHOWN_RESTRICTED:
		return _POSIX_CHOWN_RESTRICTED;
	case _PC_FILESIZEBITS:
		return (long)(sizeof(off_t) * 8);
	case _PC_LINK_MAX:
		/* no hard links; below the _POSIX_LINK_MAX floor by design */
		return 1;
	case _PC_MAX_CANON:
	case _PC_MAX_INPUT:
	case _PC_PIPE_BUF:
	case _PC_PRIO_IO:
	case _PC_SYMLINK_MAX:
	case _PC_VDISABLE:
		/* no association for this implementation */
		return -1;
	case _PC_NAME_MAX:
		return (zv != NULL && zv->f_namemax > 0) ? (long)zv->f_namemax : -1;
	case _PC_NO_TRUNC:
		return _POSIX_NO_TRUNC;
	case _PC_PATH_MAX:
		return PATH_MAX;
	case _PC_SYNC_IO:
		return IS_ENABLED(CONFIG_POSIX_SYNCHRONIZED_IO) ? 1 : -1;
	case _PC_TIMESTAMP_RESOLUTION:
		return (zv != NULL && zv->f_timestamp_resolution > 0)
			       ? (long)zv->f_timestamp_resolution
			       : -1;
	default:
		errno = EINVAL;
		return -1;
	}
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_FILE_SYSTEM_INTERNAL_H_ */

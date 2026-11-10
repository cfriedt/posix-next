/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>

#include <zephyr/fs/fs.h>
#include <zephyr/sys/zvfs_fs.h>

BUILD_ASSERT((UTIME_NOW == FS_UTIME_NOW) && (UTIME_OMIT == FS_UTIME_OMIT));

/* a marker, or nanoseconds within a second */
static bool utime_valid(const struct timespec *ts)
{
	return (ts->tv_nsec == UTIME_NOW) || (ts->tv_nsec == UTIME_OMIT) ||
	       ((ts->tv_nsec >= 0) && (ts->tv_nsec < NSEC_PER_SEC));
}

int utimensat(int fd, const char *path, const struct timespec times[2], int flag)
{
	/* the markers go to the file system, which applies them under its own lock */
	if ((times != NULL) && (!utime_valid(&times[0]) || !utime_valid(&times[1]))) {
		errno = EINVAL;
		return -1;
	}

	return zvfs_utimeat(fd, path, times, flag);
}

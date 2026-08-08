/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <time.h>
#include <utime.h>

#include <zephyr/sys/zvfs_fs.h>

int utime(const char *path, const struct utimbuf *times)
{
	struct timespec ts[2];
	const struct timespec *tp = NULL;

	if (times != NULL) {
		ts[0].tv_sec = times->actime;
		ts[0].tv_nsec = 0;
		ts[1].tv_sec = times->modtime;
		ts[1].tv_nsec = 0;
		tp = ts;
	}

	return zvfs_utime(path, tp);
}

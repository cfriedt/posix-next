/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/stat.h>
#include <time.h>

#include <zephyr/sys/zvfs_fs.h>

int utimensat(int fd, const char *path, const struct timespec times[2], int flag)
{
	struct timespec ts[2];
	struct zvfs_stat zs;
	bool stat_done = false;

	if (times == NULL) {
		return zvfs_utimeat(fd, path, NULL, flag);
	}

	for (int i = 0; i < 2; i++) {
		ts[i] = times[i];
		if (times[i].tv_nsec == UTIME_NOW) {
			if (clock_gettime(CLOCK_REALTIME, &ts[i]) < 0) {
				return -1;
			}
		} else if (times[i].tv_nsec == UTIME_OMIT) {
			/* keep the current value of the omitted timestamp */
			if (!stat_done) {
				if (zvfs_statat(fd, path, &zs, flag) < 0) {
					return -1;
				}
				stat_done = true;
			}
			ts[i] = (i == 0) ? zs.atime : zs.mtime;
		}
	}

	return zvfs_utimeat(fd, path, ts, flag);
}

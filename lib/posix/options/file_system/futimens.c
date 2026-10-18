/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#include <zephyr/sys/zvfs_fs.h>

int futimens(int fd, const struct timespec times[2])
{
	struct timespec ts[2];
	struct zvfs_stat zs;
	bool stat_done = false;

	if (times == NULL) {
		return zvfs_futime(fd, NULL);
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
				if (zvfs_fstat(fd, &zs) < 0) {
					return -1;
				}
				stat_done = true;
			}
			ts[i] = (i == 0) ? zs.atime : zs.mtime;
		} else if ((times[i].tv_nsec < 0) || (times[i].tv_nsec >= NSEC_PER_SEC)) {
			errno = EINVAL;
			return -1;
		}
	}

	return zvfs_futime(fd, ts);
}

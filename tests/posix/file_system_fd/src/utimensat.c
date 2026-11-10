/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_utimensat)
{
	int dirfd = open_test_root();
	struct timespec times[2] = {
		{.tv_sec = 1700000000, .tv_nsec = 0},
		{.tv_sec = 1700000000, .tv_nsec = 0},
	};
	int rc;

	rc = utimensat(dirfd, "file.txt", times, 0);
	if (rc < 0) {
		/* backends without timestamp support report ENOTSUP */
		zassert_equal(errno, ENOTSUP, "errno %d", errno);
	}

	/* the UTIME_NOW and UTIME_OMIT markers are honored, or unsupported */
	times[0].tv_nsec = UTIME_NOW;
	times[1].tv_nsec = UTIME_OMIT;
	rc = utimensat(dirfd, "file.txt", times, 0);
	if (rc < 0) {
		zassert_equal(errno, ENOTSUP, "errno %d", errno);
	}

	rc = utimensat(AT_FDCWD, TEST_FILE, NULL, 0);
	if (rc < 0) {
		zassert_equal(errno, ENOTSUP, "errno %d", errno);
	}

	errno = 0;
	zassert_equal(utimensat(dirfd, "nope.txt", NULL, 0), -1);
	zassert_true(errno == ENOENT || errno == ENOTSUP, "errno %d", errno);

	zassert_ok(close(dirfd));
}

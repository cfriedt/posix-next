/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_tests.h"

#include <errno.h>
#include <sys/stat.h>
#include <time.h>

ZTEST_USER(posix_file_system, test_futimens)
{
	struct timespec times[2] = {
		{.tv_sec = 1700000000, .tv_nsec = 0},
		{.tv_sec = 1700000100, .tv_nsec = 0},
	};
	struct timespec now;
	struct stat st;
	int rc;

	errno = 0;
	zassert_equal(futimens(-1, times), -1);
	zassert_equal(errno, EBADF);

	/* an out-of-range tv_nsec is rejected before the file system is asked */
	times[1].tv_nsec = NSEC_PER_SEC;
	errno = 0;
	zassert_equal(futimens(fs_test_fd, times), -1);
	zassert_equal(errno, EINVAL);
	times[1].tv_nsec = 0;

	/* times are stored as given, kept for UTIME_OMIT and resolved for UTIME_NOW */
	rc = futimens(fs_test_fd, times);
	if (rc < 0) {
		/* backends without timestamp support report ENOTSUP */
		zassert_equal(errno, ENOTSUP, "errno %d", errno);
		return;
	}
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_mtim.tv_sec, times[1].tv_sec);

	times[0].tv_nsec = UTIME_OMIT;
	times[1].tv_sec = 1700000200;
	zassert_ok(futimens(fs_test_fd, times));
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_mtim.tv_sec, 1700000200);

	zassert_ok(clock_gettime(CLOCK_REALTIME, &now));
	times[0].tv_nsec = UTIME_NOW;
	times[1].tv_nsec = UTIME_OMIT;
	zassert_ok(futimens(fs_test_fd, times));
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_equal(st.st_mtim.tv_sec, 1700000200);

	zassert_ok(futimens(fs_test_fd, NULL));
	zassert_ok(fstat(fs_test_fd, &st));
	zassert_true(st.st_mtim.tv_sec >= now.tv_sec - 2, "mtime %lld", (long long)st.st_mtim.tv_sec);
}

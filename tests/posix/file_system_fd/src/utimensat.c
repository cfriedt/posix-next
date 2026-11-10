/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* a nanosecond count that is neither a marker nor within a second is refused before any file system call */
static void utimensat_invalid(int dirfd)
{
	struct timespec times[2] = {
		{.tv_sec = 1700000000, .tv_nsec = 0},
		{.tv_sec = 1700000000, .tv_nsec = 1000000000L},
	};

	errno = 0;
	zassert_equal(utimensat(dirfd, "file.txt", times, 0), -1);
	zassert_equal(errno, EINVAL, "errno %d", errno);
	times[1].tv_nsec = 0;
	times[0].tv_nsec = -3;
	errno = 0;
	zassert_equal(utimensat(dirfd, "file.txt", times, 0), -1);
	zassert_equal(errno, EINVAL, "errno %d", errno);
}

#ifndef CONFIG_NATIVE_LIBC
/* an omitted timestamp is left as it is, under the file system's lock */
static void utimensat_omit(void)
{
	const struct timespec set[2] = {{.tv_sec = 1700000000}, {.tv_sec = 1700000100}};
	const struct timespec now_omit[2] = {{.tv_nsec = UTIME_NOW}, {.tv_nsec = UTIME_OMIT}};
	struct stat st;

	zassert_ok(utimensat(AT_FDCWD, TEST_TMPFS_FILE, set, 0));
	zassert_ok(utimensat(AT_FDCWD, TEST_TMPFS_FILE, now_omit, 0));
	zassert_ok(stat(TEST_TMPFS_FILE, &st));
	zassert_equal(st.st_mtim.tv_sec, set[1].tv_sec, "the omitted mtime changed");
	zassert_not_equal(st.st_atim.tv_sec, set[0].tv_sec, "atime was not set to now");
}

/* AT_SYMLINK_NOFOLLOW never stamps the link's target: the link itself, or an error */
static void utimensat_nofollow(void)
{
	const struct timespec set[2] = {{.tv_sec = 1700000000}, {.tv_sec = 1700000000}};
	const struct timespec other[2] = {{.tv_sec = 1600000000}, {.tv_sec = 1600000000}};
	struct stat st;
	int rc;

	zassert_ok(utimensat(AT_FDCWD, TEST_TMPFS_FILE, set, 0));

	errno = 0;
	rc = utimensat(AT_FDCWD, TEST_TMPFS_LINK, other, AT_SYMLINK_NOFOLLOW);
	zassert_true((rc == 0) || (errno == EOPNOTSUPP) || (errno == ENOTSUP), "errno %d", errno);
	errno = 0;
	rc = utimensat(AT_FDCWD, TEST_TMPFS_LINK, NULL, AT_SYMLINK_NOFOLLOW);
	zassert_true((rc == 0) || (errno == EOPNOTSUPP) || (errno == ENOTSUP), "errno %d", errno);

	zassert_ok(stat(TEST_TMPFS_FILE, &st));
	zassert_equal(st.st_mtim.tv_sec, set[1].tv_sec, "the link's target was stamped");
	zassert_equal(st.st_atim.tv_sec, set[0].tv_sec, "the link's target was stamped");
}
#endif /* !CONFIG_NATIVE_LIBC */

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

	utimensat_invalid(dirfd);
#ifndef CONFIG_NATIVE_LIBC
	utimensat_omit();
	if (IS_ENABLED(CONFIG_FILE_SYSTEM_SYMLINKS)) {
		utimensat_nofollow();
	}
#endif /* !CONFIG_NATIVE_LIBC */

	zassert_ok(close(dirfd));
}

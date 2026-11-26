/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_fd_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_attributes_fd, test_fchownat)
{
	int dirfd = open(TEST_ROOT, O_RDONLY | O_DIRECTORY);

	zassert_true(dirfd >= 0, "open(" TEST_ROOT ") failed: %d", errno);

	/* the owning identity (a host user cannot give files away), and "leave unchanged" */
	zassert_ok(fchownat(dirfd, "file.txt", IS_ENABLED(CONFIG_NATIVE_LIBC) ? (uid_t)-1 : 0,
			    IS_ENABLED(CONFIG_NATIVE_LIBC) ? (gid_t)-1 : 0, 0));
	zassert_ok(fchownat(dirfd, "file.txt", (uid_t)-1, (gid_t)-1, AT_SYMLINK_NOFOLLOW));
	zassert_ok(fchownat(AT_FDCWD, TEST_FILE, (uid_t)-1, (gid_t)-1, 0));

	errno = 0;
	zassert_equal(fchownat(dirfd, "nope.txt", (uid_t)-1, (gid_t)-1, 0), -1);
	zassert_equal(errno, ENOENT);

	errno = 0;
	zassert_equal(fchownat(fs_test_fd, "file.txt", (uid_t)-1, (gid_t)-1, 0), -1);
	zassert_equal(errno, ENOTDIR);

	errno = 0;
	zassert_equal(fchownat(dirfd, "file.txt", (uid_t)-1, (gid_t)-1, AT_REMOVEDIR), -1);
	zassert_equal(errno, EINVAL);

	/* the privileged identity may give a file to anyone; -1 keeps the other ID */
	IF_NOT_NATIVE_LIBC({
		struct stat st;

		zassert_ok(fchownat(dirfd, "file.txt", 1000, (gid_t)-1, 0));
		zassert_ok(fstatat(dirfd, "file.txt", &st, 0));
		zassert_equal(st.st_uid, 1000);
		zassert_equal(st.st_gid, 0);
		zassert_ok(fchownat(dirfd, "file.txt", (uid_t)-1, 100, 0));
		zassert_ok(stat(TEST_FILE, &st));
		zassert_equal(st.st_uid, 1000);
		zassert_equal(st.st_gid, 100);
	});

	zassert_ok(close(dirfd));
}

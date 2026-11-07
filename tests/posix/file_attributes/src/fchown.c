/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_file_attributes, test_fchown)
{

	/* the owning identity (a host user cannot give files away), and "leave unchanged" */
	zassert_ok(fchown(fs_test_fd, IS_ENABLED(CONFIG_NATIVE_LIBC) ? (uid_t)-1 : 0,
			  IS_ENABLED(CONFIG_NATIVE_LIBC) ? (gid_t)-1 : 0));
	zassert_ok(fchown(fs_test_fd, (uid_t)-1, (gid_t)-1));
	zassert_ok(fchown(fs_test_rofd, (uid_t)-1, (gid_t)-1));

	errno = 0;
	zassert_equal(fchown(-1, (uid_t)-1, (gid_t)-1), -1);
	zassert_equal(errno, EBADF);

	/* the privileged identity may give a file to anyone; both descriptors see it */
	IF_NOT_NATIVE_LIBC({
		struct stat st;

		zassert_ok(fchown(fs_test_fd, 1000, (gid_t)-1));
		zassert_ok(fstat(fs_test_rofd, &st));
		zassert_equal(st.st_uid, 1000);
		zassert_equal(st.st_gid, 0);
		zassert_ok(fchown(fs_test_rofd, (uid_t)-1, 100));
		zassert_ok(fstat(fs_test_fd, &st));
		zassert_equal(st.st_uid, 1000);
		zassert_equal(st.st_gid, 100);
	});
}

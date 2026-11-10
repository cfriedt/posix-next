/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "symbolic_links_tests.h"

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

ZTEST_USER(posix_symbolic_links, test_lchown)
{
	struct stat st;

	/* the owning identity (a host user cannot give files away), and "leave unchanged" */
	zassert_ok(lchown(TEST_FILE, IS_ENABLED(CONFIG_NATIVE_LIBC) ? (uid_t)-1 : 0,
			  IS_ENABLED(CONFIG_NATIVE_LIBC) ? (gid_t)-1 : 0));
	zassert_ok(lchown(TEST_FILE, (uid_t)-1, (gid_t)-1));

	/* the link itself is the object: a dangling link is fine, its target is not */
	zassert_ok(symlink("nope", TEST_DANGLE));
	zassert_ok(lchown(TEST_DANGLE, (uid_t)-1, (gid_t)-1));
	errno = 0;
	zassert_equal(stat(TEST_DANGLE, &st), -1);
	zassert_equal(errno, ENOENT);
	zassert_ok(unlink(TEST_DANGLE));

	errno = 0;
	zassert_equal(lchown(TEST_NOENT, (uid_t)-1, (gid_t)-1), -1);
	zassert_equal(errno, ENOENT);

	/* the privileged identity may give a link away without touching its target */
	IF_NOT_NATIVE_LIBC({
		zassert_ok(symlink("file.txt", TEST_DANGLE));
		zassert_ok(lchown(TEST_DANGLE, 1000, (gid_t)-1));
		zassert_ok(lstat(TEST_DANGLE, &st));
		zassert_equal(st.st_uid, 1000);
		zassert_equal(st.st_gid, 0);
		zassert_ok(stat(TEST_DANGLE, &st));
		zassert_equal(st.st_uid, 0, "the target kept its owner");
		zassert_ok(unlink(TEST_DANGLE));
	});
}

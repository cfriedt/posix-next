/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_file_attributes, test_chown)
{
	/* the owning identity (a host user cannot give files away), and "leave unchanged" */
	zassert_ok(chown(TEST_FILE, IS_ENABLED(CONFIG_NATIVE_LIBC) ? (uid_t)-1 : 0,
			 IS_ENABLED(CONFIG_NATIVE_LIBC) ? (gid_t)-1 : 0));
	zassert_ok(chown(TEST_FILE, (uid_t)-1, (gid_t)-1));
	zassert_ok(chown(TEST_DIR, (uid_t)-1, (gid_t)-1));

	errno = 0;
	zassert_equal(chown(TEST_NOENT, (uid_t)-1, (gid_t)-1), -1);
	zassert_equal(errno, ENOENT);

	/* only the privileged identity exists (a host root user may give files to anyone) */
	IF_NOT_NATIVE_LIBC({
		errno = 0;
		zassert_equal(chown(TEST_FILE, 1000, (gid_t)-1), -1);
		zassert_equal(errno, EINVAL);

		errno = 0;
		zassert_equal(chown(TEST_FILE, (uid_t)-1, 1000), -1);
		zassert_equal(errno, EINVAL);
	});
}

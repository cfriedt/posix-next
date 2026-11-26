/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_attributes_tests.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static mode_t create_perm(const char *path, mode_t mode)
{
	struct stat st;
	int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, mode);

	zassert_true(fd >= 0, "create %s: %d", path, errno);
	zassert_ok(fstat(fd, &st));
	zassert_ok(close(fd));
	zassert_ok(unlink(path));

	return st.st_mode & 0777;
}

ZTEST_USER(posix_file_attributes, test_umask)
{
	struct stat st;
	mode_t initial = umask(0027);

	/* the previous mask comes back, and only the file permission bits are kept */
	zassert_equal(umask(0077), 0027);
	zassert_equal(umask(S_ISUID | S_ISGID | S_ISVTX | 0777), 0077);
	zassert_equal(umask(0), 0777);

	/* the mask withholds its bits from every file and directory created */
	zassert_equal(create_perm(TEST_NOENT, 0666), 0666);
	zassert_equal(umask(0027), 0);
	zassert_equal(create_perm(TEST_NOENT, 0666), 0640);
	zassert_equal(create_perm(TEST_NOENT, 0777), 0750);
	zassert_ok(mkdir(TEST_ROOT "/masked", 0777));
	zassert_ok(stat(TEST_ROOT "/masked", &st));
	zassert_equal(st.st_mode & 0777, 0750, "0%o", st.st_mode & 0777);
	zassert_ok(rmdir(TEST_ROOT "/masked"));
	/* an existing file keeps its mode */
	zassert_equal(umask(0777), 0027);
	{
		int fd = open(TEST_FILE, O_RDWR | O_CREAT, 0777);

		zassert_true(fd >= 0);
		zassert_ok(close(fd));
	}
	zassert_ok(stat(TEST_FILE, &st));
	zassert_equal(st.st_mode & 0777, 0600, "0%o", st.st_mode & 0777);

	zassert_equal(umask(initial), 0777);
	zassert_equal(umask(initial), initial);
}

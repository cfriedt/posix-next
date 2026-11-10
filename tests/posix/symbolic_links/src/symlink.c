/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/* d_type and the DT_* values are an extension glibc hides behind _DEFAULT_SOURCE */
#define _DEFAULT_SOURCE 1

#include "symbolic_links_tests.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* the directory lists the link as one, without following it */
static void symlink_dirent_type(void)
{
	DIR *d = opendir(TEST_ROOT);
	struct dirent *e;
	bool seen = false;

	zassert_not_null(d);
	while ((e = readdir(d)) != NULL) {
		if (strcmp(e->d_name, "link") == 0) {
			seen = true;
			zassert_equal(e->d_type, DT_LNK, "d_type of a link: %d", e->d_type);
		}
	}
	zassert_true(seen, "link not listed");
	zassert_ok(closedir(d));
}

ZTEST_USER(posix_symbolic_links, test_symlink)
{
	struct stat st;
	char buf[sizeof(TEST_CONTENT)];
	int fd;

	zassert_ok(symlink(TEST_FILE, TEST_LINK));
	zassert_ok(lstat(TEST_LINK, &st));
	zassert_true(S_ISLNK(st.st_mode));
	zassert_ok(stat(TEST_LINK, &st));
	zassert_true(S_ISREG(st.st_mode));
	symlink_dirent_type();

	/* following the link reaches the file */
	fd = open(TEST_LINK, O_RDONLY);
	zassert_true(fd >= 0, "open(link) failed: %d", errno);
	zassert_equal(read(fd, buf, sizeof(buf) - 1), strlen(TEST_CONTENT));
	zassert_ok(close(fd));

	errno = 0;
	zassert_equal(symlink(TEST_FILE, TEST_LINK), -1);
	zassert_equal(errno, EEXIST);

	/* removing the link leaves the file */
	zassert_ok(unlink(TEST_LINK));
	zassert_ok(stat(TEST_FILE, &st));
	zassert_equal(st.st_size, strlen(TEST_CONTENT));

	/* the target need not exist */
	zassert_ok(symlink("nope", TEST_DANGLE));
	zassert_ok(lstat(TEST_DANGLE, &st));
	zassert_true(S_ISLNK(st.st_mode));
	errno = 0;
	zassert_equal(stat(TEST_DANGLE, &st), -1);
	zassert_equal(errno, ENOENT);
	zassert_ok(unlink(TEST_DANGLE));

	errno = 0;
	zassert_equal(symlink(TEST_FILE, TEST_NOENT "/link"), -1);
	zassert_equal(errno, ENOENT);
}

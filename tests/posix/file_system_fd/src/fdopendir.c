/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "file_system_fd_common.h"

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

ZTEST_USER(posix_file_system_fd, test_fdopendir)
{
	int dirfd = open_test_root();
	DIR *dir;
	struct dirent *ent;
	bool found = false;

	dir = fdopendir(dirfd);
	zassert_not_null(dir, "fdopendir failed: %d", errno);

	while ((ent = readdir(dir)) != NULL) {
		/* FAT without long names reports upper case */
		if (strcasecmp(ent->d_name, "file.txt") == 0) {
			found = true;
		}
	}
	zassert_true(found, "file.txt not listed");

	/* the stream owns the descriptor: closing it closes the descriptor */
	zassert_ok(closedir(dir));
	errno = 0;
	zassert_equal(close(dirfd), -1);
	zassert_equal(errno, EBADF);

	errno = 0;
	zassert_is_null(fdopendir(fs_test_fd));
	zassert_equal(errno, ENOTDIR);

	errno = 0;
	zassert_is_null(fdopendir(-1));
	zassert_equal(errno, EBADF);
}

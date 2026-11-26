/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <dirent.h>
#include <errno.h>

#include <zephyr/sys/zvfs_fs.h>

ZTEST_USER(xsi_file_system, test_telldir)
{
	DIR *d = zvfs_opendir(TEST_DIR);

	zassert_not_null(d);

	/* the position counts the entries read; a rewind resets it */
	zassert_equal(telldir(d), 0);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 0);
	zassert_equal(telldir(d), 1);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 0);
	zassert_equal(telldir(d), 2);
	zassert_ok(zvfs_rewinddir(d->fd));
	zassert_equal(telldir(d), 0);
	zassert_ok(zvfs_close(d->fd));

	errno = 0;
	zassert_equal(telldir(NULL), -1);
	zassert_equal(errno, EBADF);
}

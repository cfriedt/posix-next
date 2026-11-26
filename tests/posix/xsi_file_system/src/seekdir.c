/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <dirent.h>
#include <string.h>

#include <zephyr/sys/zvfs_fs.h>

ZTEST_USER(xsi_file_system, test_seekdir)
{
	char first[ZVFS_NAME_MAX + 1];
	char second[ZVFS_NAME_MAX + 1];
	DIR *d = zvfs_opendir(TEST_DIR);

	zassert_not_null(d);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 0);
	strcpy(first, d->ent.d_name);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 0);
	strcpy(second, d->ent.d_name);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 1);

	/* back to a reported position, then forward again; past the end reads nothing */
	seekdir(d, 0);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 0);
	zassert_str_equal(d->ent.d_name, first);
	seekdir(d, 1);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 0);
	zassert_str_equal(d->ent.d_name, second);
	seekdir(d, 0);
	seekdir(d, 2);
	zassert_equal(zvfs_readdir(d->fd, &d->ent), 1);
	zassert_ok(zvfs_close(d->fd));

	/* no stream: nothing happens */
	seekdir(NULL, 0);
}

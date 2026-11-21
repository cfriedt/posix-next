/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <mntent.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/ztest.h>

/* the volume the suite mounts; the host's table is whatever the host has mounted */
#define TEST_MOUNT "/RAM:"

ZTEST_USER(posix_non_portable, test_setmntent)
{
	FILE *fp;

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		errno = 0;
		zassert_is_null(setmntent(NULL, "r"));
		zassert_equal(errno, EINVAL);
		errno = 0;
		zassert_is_null(setmntent(MOUNTED, "w"));
		zassert_equal(errno, EROFS);
	}

	fp = setmntent(MOUNTED, "r");
	zassert_not_null(fp);
	zassert_equal(endmntent(fp), 1);
}

ZTEST_USER(posix_non_portable, test_getmntent)
{
	FILE *fp = setmntent(MOUNTED, "r");
	struct mntent *me;
	bool saw_test_mount = false;
	int n = 0;

	zassert_not_null(fp);
	while ((me = getmntent(fp)) != NULL) {
		n++;
		zassert_not_null(me->mnt_fsname);
		zassert_not_null(me->mnt_dir);
		zassert_not_null(me->mnt_type);
		zassert_not_null(me->mnt_opts);
		zassert_equal(me->mnt_dir[0], '/', "mount point %s is not absolute", me->mnt_dir);
		zassert_true(strlen(me->mnt_type) > 0, "entry without a type");
		if (strcmp(me->mnt_dir, TEST_MOUNT) == 0) {
			saw_test_mount = true;
			zassert_str_equal(me->mnt_type, "vfat");
			zassert_str_equal(me->mnt_fsname, "vfat");
			zassert_not_null(hasmntopt(me, "rw"));
			zassert_is_null(hasmntopt(me, "ro"));
			zassert_equal(me->mnt_freq, 0);
			zassert_equal(me->mnt_passno, 0);
		}
	}
	zassert_true(n > 0, "no mounted file system listed");
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		zassert_true(saw_test_mount, "the suite's volume is not listed");
	}
	/* the end of the table stays the end */
	zassert_is_null(getmntent(fp));
	zassert_equal(endmntent(fp), 1);
}

ZTEST_USER(posix_non_portable, test_endmntent)
{
	FILE *fp = setmntent(MOUNTED, "r");

	zassert_not_null(fp);
	zassert_equal(endmntent(fp), 1);
}

ZTEST_USER(posix_non_portable, test_hasmntopt)
{
	char opts[] = "rw,nosuid,size=64k";
	struct mntent me = {.mnt_opts = opts};

	zassert_equal(hasmntopt(&me, "rw"), opts);
	zassert_equal(hasmntopt(&me, "nosuid"), opts + 3);
	zassert_equal(hasmntopt(&me, "size"), opts + 10);
	zassert_is_null(hasmntopt(&me, "ro"));
	zassert_is_null(hasmntopt(&me, "suid"));
	zassert_is_null(hasmntopt(&me, "siz"));
	zassert_is_null(hasmntopt(&me, "size=64k,"));
}

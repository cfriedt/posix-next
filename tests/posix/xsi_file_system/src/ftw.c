/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xsi_file_system_tests.h"

#include <errno.h>
#include <ftw.h>
#include <libgen.h>
#include <limits.h>
#include <string.h>

#include <zephyr/sys/zvfs_fs.h>

struct walk_entry {
	char path[48];
	int flag;
	int level;
	int base;
};

ZTEST_BMEM static struct walk_entry seen[TEST_TREE_OBJECTS + 1];
ZTEST_BMEM static int nseen;
ZTEST_BMEM static int stop_after;
ZTEST_BMEM static int cwd_mismatches;

static void walk_reset(int stop)
{
	memset(seen, 0, sizeof(seen));
	nseen = 0;
	stop_after = stop;
	cwd_mismatches = 0;
}

static int walk_record(const char *path, int flag, int level, int base)
{
	if (nseen < (int)ARRAY_SIZE(seen)) {
		strncpy(seen[nseen].path, path, sizeof(seen[nseen].path) - 1);
		seen[nseen].flag = flag;
		seen[nseen].level = level;
		seen[nseen].base = base;
	}
	nseen++;

	return ((stop_after != 0) && (nseen >= stop_after)) ? 7 : 0;
}

static int ftw_cb(const char *path, const struct stat *st, int flag)
{
	ARG_UNUSED(st);

	return walk_record(path, flag, -1, -1);
}

static int nftw_cb(const char *path, const struct stat *st, int flag, struct FTW *ftwbuf)
{
	ARG_UNUSED(st);

	return walk_record(path, flag, ftwbuf->level, ftwbuf->base);
}

/* FTW_CHDIR: the working directory is the one holding the reported object */
static int chdir_cb(const char *path, const struct stat *st, int flag, struct FTW *ftwbuf)
{
	char cwd[PATH_MAX];
	char parent[PATH_MAX];

	ARG_UNUSED(st);

	strcpy(parent, path);
	if ((zvfs_getcwd(cwd, sizeof(cwd)) < 0) || (strcmp(cwd, dirname(parent)) != 0)) {
		cwd_mismatches++;
	}

	return walk_record(path, flag, ftwbuf->level, ftwbuf->base);
}

static const struct walk_entry *walk_find(const char *path)
{
	for (int i = 0; i < MIN(nseen, (int)ARRAY_SIZE(seen)); i++) {
		if (strcmp(seen[i].path, path) == 0) {
			return &seen[i];
		}
	}

	return NULL;
}

static void walk_expect(const char *path, int flag, int level)
{
	const struct walk_entry *e = walk_find(path);

	zassert_not_null(e, "%s not reported", path);
	zassert_equal(e->flag, flag, "%s: flag %d", path, e->flag);
	if (level >= 0) {
		zassert_equal(e->level, level, "%s: level %d", path, e->level);
		zassert_equal(e->base, (int)(strrchr(path, '/') - path) + 1, "%s: base %d", path,
			      e->base);
	}
}

/* the fixture tree, links followed */
static void walk_expect_tree(int level_known)
{
	int l = level_known ? 0 : -1;

	zassert_equal(nseen, TEST_TREE_OBJECTS, "%d objects reported", nseen);
	walk_expect(TEST_ROOT, FTW_D, l);
	walk_expect(TEST_FILE, FTW_F, level_known ? 1 : -1);
	walk_expect(TEST_DIR, FTW_D, level_known ? 1 : -1);
	walk_expect(TEST_SUB, FTW_F, level_known ? 2 : -1);
	walk_expect(TEST_DEEP, FTW_D, level_known ? 2 : -1);
	walk_expect(TEST_LEAF, FTW_F, level_known ? 3 : -1);
	walk_expect(TEST_EMPTY, FTW_D, level_known ? 1 : -1);
	walk_expect(TEST_LINK, FTW_F, level_known ? 1 : -1);
	walk_expect(TEST_DANGLING, FTW_SLN, level_known ? 1 : -1);
}

ZTEST_USER(xsi_file_system, test_ftw)
{
	walk_reset(0);
	zassert_ok(ftw(TEST_ROOT, ftw_cb, 8));
	walk_expect_tree(false);

	/* a file as the starting point */
	walk_reset(0);
	zassert_ok(ftw(TEST_FILE, ftw_cb, 8));
	zassert_equal(nseen, 1);
	walk_expect(TEST_FILE, FTW_F, -1);

	/* the callback's nonzero return ends the walk and is returned */
	walk_reset(3);
	zassert_equal(ftw(TEST_ROOT, ftw_cb, 8), 7);
	zassert_equal(nseen, 3);

	errno = 0;
	zassert_equal(ftw(TEST_NOENT, ftw_cb, 8), -1);
	zassert_equal(errno, ENOENT);
	errno = 0;
	zassert_equal(ftw(TEST_ROOT, ftw_cb, 0), -1);
	zassert_equal(errno, EINVAL);
	errno = 0;
	zassert_equal(ftw(TEST_ROOT, NULL, 8), -1);
	zassert_equal(errno, EINVAL);
}

ZTEST_USER(xsi_file_system, test_nftw)
{
	const struct walk_entry *e;

	walk_reset(0);
	zassert_ok(nftw(TEST_ROOT, nftw_cb, 8, 0));
	walk_expect_tree(true);

	/* FTW_PHYS reports the links themselves */
	walk_reset(0);
	zassert_ok(nftw(TEST_ROOT, nftw_cb, 8, FTW_PHYS));
	zassert_equal(nseen, TEST_TREE_OBJECTS);
	walk_expect(TEST_LINK, FTW_SL, 1);
	walk_expect(TEST_DANGLING, FTW_SL, 1);

	/* FTW_DEPTH reports a directory after its contents */
	walk_reset(0);
	zassert_ok(nftw(TEST_DIR, nftw_cb, 8, FTW_DEPTH));
	zassert_equal(nseen, 4);
	walk_expect(TEST_DIR, FTW_DP, 0);
	walk_expect(TEST_DEEP, FTW_DP, 1);
	e = &seen[nseen - 1];
	zassert_str_equal(e->path, TEST_DIR);
	zassert_true(walk_find(TEST_LEAF) < walk_find(TEST_DEEP), "deep before its leaf");

	/* one descriptor: directories are given up and returned to */
	walk_reset(0);
	zassert_ok(nftw(TEST_ROOT, nftw_cb, 1, 0));
	walk_expect_tree(true);

	/* FTW_MOUNT: one file system here, nothing is skipped */
	walk_reset(0);
	zassert_ok(nftw(TEST_ROOT, nftw_cb, 8, FTW_MOUNT));
	zassert_equal(nseen, TEST_TREE_OBJECTS);

	/* FTW_CHDIR: the callback runs in the object's directory; the caller's is restored */
	zassert_ok(zvfs_chdir(TEST_EMPTY));
	walk_reset(0);
	zassert_ok(nftw(TEST_ROOT, chdir_cb, 8, FTW_CHDIR | FTW_PHYS));
	zassert_equal(nseen, TEST_TREE_OBJECTS);
	zassert_equal(cwd_mismatches, 0);
	{
		char cwd[PATH_MAX];

		zassert_ok(zvfs_getcwd(cwd, sizeof(cwd)));
		zassert_str_equal(cwd, TEST_EMPTY);
	}
	zassert_ok(zvfs_chdir("/"));

	/* the callback's nonzero return ends the walk and is returned */
	walk_reset(3);
	zassert_equal(nftw(TEST_ROOT, nftw_cb, 8, 0), 7);
	zassert_equal(nseen, 3);

	errno = 0;
	zassert_equal(nftw(TEST_NOENT, nftw_cb, 8, 0), -1);
	zassert_equal(errno, ENOENT);
	errno = 0;
	zassert_equal(nftw(TEST_ROOT, nftw_cb, 0, 0), -1);
	zassert_equal(errno, EINVAL);
	errno = 0;
	zassert_equal(nftw(TEST_ROOT, nftw_cb, 8, 0x100), -1);
	zassert_equal(errno, EINVAL);
	errno = 0;
	zassert_equal(nftw("", nftw_cb, 8, 0), -1);
	zassert_equal(errno, ENOENT);
}

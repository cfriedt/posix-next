/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_stat.h"

#include <errno.h>
#include <ftw.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

struct ftw_walk {
	int (*nfn)(const char *path, const struct stat *st, int flag, struct FTW *ftwbuf);
	int (*fn)(const char *path, const struct stat *st, int flag);
	int fd_limit;
	int flags;
	dev_t dev;
	char path[PATH_MAX];
};

/* FTW_CHDIR: the callback runs in the directory that holds the reported object */
static int ftw_enter_parent(struct ftw_walk *w, int base)
{
	char saved;
	int rc;

	if (((w->flags & FTW_CHDIR) == 0) || (base == 0)) {
		return 0;
	}
	/* the parent is the path up to the name; the root keeps its slash */
	base = (base > 1) ? base - 1 : 1;
	saved = w->path[base];
	w->path[base] = '\0';
	rc = zvfs_chdir(w->path);
	w->path[base] = saved;

	return rc;
}

static int ftw_report(struct ftw_walk *w, const struct stat *st, int flag, int level, int base)
{
	struct FTW ftwbuf = {.base = base, .level = level};

	if (ftw_enter_parent(w, base) < 0) {
		return -1;
	}
	if (w->nfn != NULL) {
		return w->nfn(w->path, st, flag, &ftwbuf);
	}

	return w->fn(w->path, st, flag);
}

/* the FTW_* type of the object at w->path, or -1 for a directory to descend into */
static int ftw_classify(struct ftw_walk *w, struct stat *st)
{
	struct zvfs_stat zs;
	bool phys = (w->flags & FTW_PHYS) != 0;

	if ((phys ? zvfs_lstat(w->path, &zs) : zvfs_stat(w->path, &zs)) < 0) {
		if (!phys && (zvfs_lstat(w->path, &zs) == 0) &&
		    ((zs.mode & ZVFS_MODE_IFMT) == ZVFS_MODE_IFLNK)) {
			posix_stat_from_zvfs(st, &zs);
			return FTW_SLN;
		}
		return FTW_NS;
	}
	posix_stat_from_zvfs(st, &zs);
	if (S_ISLNK(st->st_mode)) {
		return FTW_SL;
	}
	if (S_ISDIR(st->st_mode)) {
		return -1;
	}

	return FTW_F;
}

static int ftw_walk(struct ftw_walk *w, int level)
{
	struct stat st;
	struct zvfs_dirstream *d;
	size_t len = strlen(w->path);
	const char *slash = strrchr(w->path, '/');
	int base = (slash == NULL) ? 0 : (int)(slash - w->path) + 1;
	int flag = ftw_classify(w, &st);
	long pos = 0;
	int rc;

	if (flag != -1) {
		if ((level == 0) && (flag == FTW_NS)) {
			return -1;
		}
		return ftw_report(w, &st, flag, level, base);
	}

	if (level == 0) {
		w->dev = st.st_dev;
	} else if (((w->flags & FTW_MOUNT) != 0) && (st.st_dev != w->dev)) {
		return 0;
	}

	if ((w->flags & FTW_DEPTH) == 0) {
		rc = ftw_report(w, &st, FTW_D, level, base);
		if (rc != 0) {
			return rc;
		}
	}

	d = zvfs_opendir(w->path);
	if (d == NULL) {
		return ftw_report(w, &st, FTW_DNR, level, base);
	}

	/* a trailing slash on the starting path is not doubled */
	if (w->path[len - 1] == '/') {
		len--;
	}

	for (;;) {
		const char *name;
		bool reopen;

		rc = zvfs_readdir(d->fd, &d->ent);
		if (rc != 0) {
			rc = (rc < 0) ? -1 : 0;
			break;
		}
		name = d->ent.d_name;
		if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0)) {
			continue;
		}
		if (len + 1 + strlen(name) >= sizeof(w->path)) {
			errno = ENAMETOOLONG;
			rc = -1;
			break;
		}
		w->path[len] = '/';
		strcpy(w->path + len + 1, name);

		/* within fd_limit descriptors: give this one up and come back to its position */
		reopen = (level + 1 >= w->fd_limit);
		if (reopen) {
			pos = zvfs_telldir(d->fd);
			(void)zvfs_close(d->fd);
			d = NULL;
		}
		rc = ftw_walk(w, level + 1);
		w->path[len] = '\0';
		if (rc != 0) {
			break;
		}
		if (reopen) {
			d = zvfs_opendir(w->path);
			if ((d == NULL) || (zvfs_seekdir(d->fd, pos) < 0)) {
				rc = -1;
				break;
			}
		}
	}
	if (d != NULL) {
		(void)zvfs_close(d->fd);
	}
	if (rc != 0) {
		return rc;
	}
	if ((w->flags & FTW_DEPTH) != 0) {
		return ftw_report(w, &st, FTW_DP, level, base);
	}

	return 0;
}

static int ftw_run(struct ftw_walk *w, const char *path)
{
	char cwd[PATH_MAX];
	int rc;

	if ((path == NULL) || (w->fd_limit <= 0) ||
	    ((w->flags & ~(FTW_PHYS | FTW_MOUNT | FTW_DEPTH | FTW_CHDIR)) != 0)) {
		errno = EINVAL;
		return -1;
	}
	if (path[0] == '\0') {
		errno = ENOENT;
		return -1;
	}
	if (strlen(path) >= sizeof(w->path)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcpy(w->path, path);

	if (((w->flags & FTW_CHDIR) != 0) && (zvfs_getcwd(cwd, sizeof(cwd)) < 0)) {
		return -1;
	}
	rc = ftw_walk(w, 0);
	if ((w->flags & FTW_CHDIR) != 0) {
		int err = errno;

		(void)zvfs_chdir(cwd);
		errno = err;
	}

	return rc;
}

int ftw(const char *path, int (*fn)(const char *path, const struct stat *st, int flag),
	int ndirs)
{
	struct ftw_walk w = {.fn = fn, .fd_limit = ndirs};

	if (fn == NULL) {
		errno = EINVAL;
		return -1;
	}

	return ftw_run(&w, path);
}

int nftw(const char *path,
	 int (*fn)(const char *path, const struct stat *st, int flag, struct FTW *ftwbuf),
	 int fd_limit, int flags)
{
	struct ftw_walk w = {.nfn = fn, .fd_limit = fd_limit, .flags = flags};

	if (fn == NULL) {
		errno = EINVAL;
		return -1;
	}

	return ftw_run(&w, path);
}

/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <glob.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* the BSD header Newlib and Picolibc ship spells these differently or not at all */
#ifndef GLOB_ABORTED
#define GLOB_ABORTED GLOB_ABEND
#endif
#ifndef GLOB_NOESCAPE
#define GLOB_NOESCAPE 0x2000
#endif

struct glob_walk {
	glob_t *pglob;
	int flags;
	int (*errfunc)(const char *epath, int eerrno);
	int start;
};

static bool glob_is_dir(const char *path)
{
	struct stat st;

	return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
}

/* does the segment contain an unescaped metacharacter? */
static bool glob_has_meta(const char *seg, size_t len, int flags)
{
	for (size_t i = 0; i < len; i++) {
		if ((seg[i] == '\\') && ((flags & GLOB_NOESCAPE) == 0)) {
			i++;
			continue;
		}
		if ((seg[i] == '*') || (seg[i] == '?') || (seg[i] == '[')) {
			return true;
		}
	}

	return false;
}

/* copy a segment, dropping the escaping backslashes */
static void glob_unescape(const char *seg, size_t len, int flags, char *out)
{
	for (size_t i = 0; i < len; i++) {
		if ((seg[i] == '\\') && ((flags & GLOB_NOESCAPE) == 0) && ((i + 1) < len)) {
			i++;
		}
		*out++ = seg[i];
	}
	*out = '\0';
}

static int glob_append_path(struct glob_walk *gw, const char *path, bool slash)
{
	glob_t *pglob = gw->pglob;
	size_t offs = ((gw->flags & GLOB_DOOFFS) != 0) ? (size_t)pglob->gl_offs : 0;
	size_t used = offs + (size_t)pglob->gl_pathc;
	size_t len = strlen(path);
	bool mark = slash && (len > 0) && (path[len - 1] != '/');
	char **vec;
	char *copy;

	vec = realloc(pglob->gl_pathv, (used + 2) * sizeof(char *));
	if (vec == NULL) {
		return GLOB_NOSPACE;
	}
	if (pglob->gl_pathv == NULL) {
		for (size_t i = 0; i < offs; i++) {
			vec[i] = NULL;
		}
	}
	pglob->gl_pathv = vec;

	copy = malloc(len + (mark ? 1 : 0) + 1);
	if (copy == NULL) {
		return GLOB_NOSPACE;
	}
	(void)memcpy(copy, path, len);
	if (mark) {
		copy[len++] = '/';
	}
	copy[len] = '\0';

	vec[used] = copy;
	vec[used + 1] = NULL;
	pglob->gl_pathc++;
	pglob->gl_matchc++;

	return 0;
}

/*
 * Match one pattern segment against the entries of the directory named by
 * prefix (the current working directory when empty), recursing into matching
 * directories for the remaining segments.
 */
static int glob_walk_segment(struct glob_walk *gw, const char *prefix, const char *rest)
{
	const char *seg = rest;
	const char *end;
	size_t seglen;
	int ret = 0;

	/* successive slashes collapse */
	while (*seg == '/') {
		seg++;
	}
	end = strchr(seg, '/');
	seglen = (end == NULL) ? strlen(seg) : (size_t)(end - seg);
	rest = (end == NULL) ? "" : end;

	if (seglen == 0) {
		/* the pattern ended in '/': only a directory matches, and the slash stays */
		if (glob_is_dir((prefix[0] == '\0') ? "." : prefix)) {
			return glob_append_path(gw, prefix, true);
		}
		return 0;
	}

	size_t prefix_len = strlen(prefix);
	bool last = (rest[0] == '\0');
	char *path = malloc(prefix_len + 1 + seglen + 1);

	if (path == NULL) {
		return GLOB_NOSPACE;
	}
	(void)memcpy(path, prefix, prefix_len);
	if ((prefix_len > 0) && (prefix[prefix_len - 1] != '/')) {
		path[prefix_len++] = '/';
	}

	if (!glob_has_meta(seg, seglen, gw->flags)) {
		/* literal segment: descend without reading the directory */
		glob_unescape(seg, seglen, gw->flags, &path[prefix_len]);

		if (!last) {
			ret = glob_walk_segment(gw, path, rest);
		} else {
			struct stat st;

			if (stat(path, &st) == 0) {
				bool mark = ((gw->flags & GLOB_MARK) != 0) && S_ISDIR(st.st_mode);

				ret = glob_append_path(gw, path, mark);
			}
		}
		free(path);
		return ret;
	}

	char *pat = malloc(seglen + 1);

	if (pat == NULL) {
		free(path);
		return GLOB_NOSPACE;
	}
	(void)memcpy(pat, seg, seglen);
	pat[seglen] = '\0';

	const char *dirpath = (prefix[0] == '\0') ? "." : prefix;
	DIR *dir = opendir(dirpath);

	if (dir == NULL) {
		int eerrno = errno;

		free(pat);
		free(path);
		if (((gw->errfunc != NULL) && (gw->errfunc(dirpath, eerrno) != 0)) ||
		    ((gw->flags & GLOB_ERR) != 0)) {
			return GLOB_ABORTED;
		}
		return 0;
	}

	int fnm_flags = FNM_PERIOD | (((gw->flags & GLOB_NOESCAPE) != 0) ? FNM_NOESCAPE : 0);
	struct dirent *de;

	while ((ret == 0) && ((de = readdir(dir)) != NULL)) {
		if ((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0)) {
			/* only an explicit dot pattern matches these */
			if (!((pat[0] == '.') && (fnmatch(pat, de->d_name, fnm_flags) == 0))) {
				continue;
			}
		} else if (fnmatch(pat, de->d_name, fnm_flags) != 0) {
			continue;
		}

		size_t namelen = strlen(de->d_name);
		char *match = realloc(path, prefix_len + namelen + 1);

		if (match == NULL) {
			ret = GLOB_NOSPACE;
			break;
		}
		path = match;
		(void)memcpy(&path[prefix_len], de->d_name, namelen + 1);

		if (last) {
			ret = glob_append_path(gw, path,
					       ((gw->flags & GLOB_MARK) != 0) && glob_is_dir(path));
		} else if (glob_is_dir(path)) {
			ret = glob_walk_segment(gw, path, rest);
		}
	}

	(void)closedir(dir);
	free(pat);
	free(path);

	return ret;
}

static int glob_compar(const void *a, const void *b)
{
	return strcmp(*(char *const *)a, *(char *const *)b);
}

int glob(const char *__restrict pattern, int flags,
	 int (*errfunc)(const char *epath, int eerrno), glob_t *__restrict pglob)
{
	struct glob_walk gw = {
		.pglob = pglob,
		.flags = flags,
		.errfunc = errfunc,
	};
	size_t offs;
	int ret;

	if ((pattern == NULL) || (pglob == NULL)) {
		return GLOB_ABORTED;
	}

	if ((flags & GLOB_APPEND) == 0) {
		pglob->gl_pathc = 0;
		pglob->gl_pathv = NULL;
		if ((flags & GLOB_DOOFFS) == 0) {
			pglob->gl_offs = 0;
		}
	}
	pglob->gl_flags = flags;
	pglob->gl_errfunc = errfunc;
	pglob->gl_matchc = 0;
	gw.start = pglob->gl_pathc;

	ret = glob_walk_segment(&gw, (pattern[0] == '/') ? "/" : "", pattern);
	if (ret != 0) {
		return ret;
	}

	offs = ((flags & GLOB_DOOFFS) != 0) ? (size_t)pglob->gl_offs : 0;

	if (pglob->gl_matchc == 0) {
		if ((flags & GLOB_NOCHECK) == 0) {
			return GLOB_NOMATCH;
		}
		return glob_append_path(&gw, pattern, false);
	}

	if ((flags & GLOB_NOSORT) == 0) {
		qsort(&pglob->gl_pathv[offs + gw.start], pglob->gl_matchc, sizeof(char *),
		      glob_compar);
	}

	return 0;
}

void globfree(glob_t *pglob)
{
	size_t offs;

	if ((pglob == NULL) || (pglob->gl_pathv == NULL)) {
		return;
	}

	offs = ((pglob->gl_flags & GLOB_DOOFFS) != 0) ? (size_t)pglob->gl_offs : 0;

	for (int i = 0; i < pglob->gl_pathc; i++) {
		free(pglob->gl_pathv[offs + i]);
	}
	free(pglob->gl_pathv);
	pglob->gl_pathv = NULL;
	pglob->gl_pathc = 0;
}

/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>

/* append "/name" to out, which holds len characters */
static int path_append(char *out, size_t *len, const char *name, size_t nlen)
{
	size_t sep = ((*len == 1) && (out[0] == '/')) ? 0 : 1;

	if (*len + sep + nlen >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (sep != 0) {
		out[(*len)++] = '/';
	}
	memcpy(out + *len, name, nlen);
	*len += nlen;
	out[*len] = '\0';

	return 0;
}

/* drop the last component of out, never going above the root */
static void path_parent(char *out, size_t *len)
{
	while ((*len > 1) && (out[*len - 1] != '/')) {
		(*len)--;
	}
	if (*len > 1) {
		(*len)--;
	}
	out[*len] = '\0';
}

char *realpath(const char *ZRESTRICT file_name, char *ZRESTRICT resolved_name)
{
	char work[PATH_MAX];
	char out[PATH_MAX];
	char *rest;
	size_t len;
	int links = 0;

	if (file_name == NULL) {
		errno = EINVAL;
		return NULL;
	}
	if (file_name[0] == '\0') {
		errno = ENOENT;
		return NULL;
	}
	if (strlen(file_name) >= sizeof(work)) {
		errno = ENAMETOOLONG;
		return NULL;
	}
	strcpy(work, file_name);
	rest = work;

	if (file_name[0] == '/') {
		strcpy(out, "/");
	} else if (zvfs_getcwd(out, sizeof(out)) < 0) {
		return NULL;
	}
	len = strlen(out);

	while (*rest != '\0') {
		struct zvfs_stat st;
		char *comp;
		size_t clen;

		while (*rest == '/') {
			rest++;
		}
		if (*rest == '\0') {
			break;
		}
		comp = rest;
		while ((*rest != '\0') && (*rest != '/')) {
			rest++;
		}
		clen = rest - comp;

		if ((clen == 1) && (comp[0] == '.')) {
			continue;
		}
		if ((clen == 2) && (comp[0] == '.') && (comp[1] == '.')) {
			path_parent(out, &len);
			continue;
		}
		if (path_append(out, &len, comp, clen) < 0) {
			return NULL;
		}
		if (zvfs_lstat(out, &st) < 0) {
			return NULL;
		}
		if ((st.mode & ZVFS_MODE_IFMT) == ZVFS_MODE_IFLNK) {
			char link[PATH_MAX];
			ssize_t n;

			if (++links > SYMLOOP_MAX) {
				errno = ELOOP;
				return NULL;
			}
			n = zvfs_readlink(out, link, sizeof(link) - 1);
			if (n < 0) {
				return NULL;
			}
			link[n] = '\0';
			/* the link's target, then whatever followed the link */
			if ((size_t)n + 1 + strlen(rest) >= sizeof(work)) {
				errno = ENAMETOOLONG;
				return NULL;
			}
			memmove(work + n + 1, rest, strlen(rest) + 1);
			memcpy(work, link, n);
			work[n] = '/';
			rest = work;
			if (link[0] == '/') {
				strcpy(out, "/");
				len = 1;
			} else {
				path_parent(out, &len);
			}
		} else if ((st.mode & ZVFS_MODE_IFMT) != ZVFS_MODE_IFDIR) {
			const char *more = rest;

			while (*more == '/') {
				more++;
			}
			if (*more != '\0') {
				errno = ENOTDIR;
				return NULL;
			}
		}
	}

	if (resolved_name == NULL) {
		resolved_name = malloc(PATH_MAX);
		if (resolved_name == NULL) {
			errno = ENOMEM;
			return NULL;
		}
	}
	strcpy(resolved_name, out);

	return resolved_name;
}

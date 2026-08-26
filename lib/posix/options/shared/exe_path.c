/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>

#include "posix_internal.h"

/* collapse "." and ".." components and repeated slashes of an absolute path, in place */
static void exe_path_normalize(char *path)
{
	char *out = path;
	const char *in = path;

	while (*in != '\0') {
		const char *seg;
		size_t len;

		while (*in == '/') {
			in++;
		}
		seg = in;
		while ((*in != '\0') && (*in != '/')) {
			in++;
		}
		len = (size_t)(in - seg);
		if ((len == 0) || ((len == 1) && (seg[0] == '.'))) {
			continue;
		}
		if ((len == 2) && (seg[0] == '.') && (seg[1] == '.')) {
			while ((out > path) && (*(out - 1) != '/')) {
				out--;
			}
			if (out > path) {
				out--;
			}
			continue;
		}
		*out++ = '/';
		memmove(out, seg, len);
		out += len;
	}
	if (out == path) {
		*out++ = '/';
	}
	*out = '\0';
}

const char *z_posix_exe_path(const char *file, char *buf, size_t buf_size, int *err)
{
	const char *paths;
	int fd;

	if (file == NULL) {
		*err = EINVAL;
		return NULL;
	}
	if (file[0] == '\0') {
		*err = ENOENT;
		return NULL;
	}
	if (strchr(file, '/') != NULL) {
		return file;
	}

	/* the minimal C library has an environment only through the POSIX layer */
#if !defined(CONFIG_MINIMAL_LIBC) || defined(CONFIG_POSIX_SINGLE_PROCESS) || \
	defined(CONFIG_XSI_SINGLE_PROCESS)
	paths = getenv("PATH");
#else
	paths = NULL;
#endif
	if (paths == NULL) {
		paths = CONFIG_POSIX_DEFAULT_PATH;
	}
	fd = zvfs_exe_resolve(paths, file, buf, buf_size);
	if (fd < 0) {
		switch (fd) {
		case -EPERM:
			*err = EACCES;
			break;
		case -ERANGE:
			*err = ENAMETOOLONG;
			break;
		default:
			*err = -fd;
			break;
		}
		return NULL;
	}
	/* the descriptor proved the file; the image is loaded by name */
	(void)zvfs_close(fd);

	/*
	 * The loader takes the path as given, so one from a relative entry is anchored to
	 * the working directory and normalized ("." and ".." components, repeated slashes).
	 */
	if (buf[0] != '/') {
		char cwd[PATH_MAX];
		size_t cwdlen;
		size_t rellen = strlen(buf);

		if (zvfs_getcwd(cwd, sizeof(cwd)) < 0) {
			*err = errno;
			return NULL;
		}
		cwdlen = strlen(cwd);
		if (cwdlen + 1 + rellen + 1 > buf_size) {
			*err = ENAMETOOLONG;
			return NULL;
		}
		memmove(buf + cwdlen + 1, buf, rellen + 1);
		memcpy(buf, cwd, cwdlen);
		buf[cwdlen] = '/';
		exe_path_normalize(buf);
	}

	return buf;
}

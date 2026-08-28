/*
 * Copyright (c) 2024 Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/sys/zvfs_fs.h>
#include <zephyr/posix/grp.h>
#include <zephyr/posix/pwd.h>

static int count(const char *s, char c)
{
	int count;

	for (count = 0; *s != '\0'; ++s) {
		count += (*s == c) ? 1 : 0;
	}

	return count;
}

/* fgets()-like line reader over zvfs syscalls, usable from user mode */
static ssize_t read_line(int fd, char *buf, size_t bufsize)
{
	char *nl;
	ssize_t n = zvfs_read_offset(fd, buf, bufsize - 1, NULL);

	if (n <= 0) {
		return n;
	}
	buf[n] = '\0';

	nl = strchr(buf, '\n');
	if (nl != NULL) {
		ssize_t used = (nl - buf) + 1;

		if ((used < n) && (zvfs_lseek(fd, used - n, ZVFS_SEEK_CUR) < 0)) {
			return -1;
		}
		nl[1] = '\0';
		return used;
	}

	return n;
}

int z_getgr_r(const char *name, gid_t gid, struct group *grp, char *buffer, size_t bufsize,
	      struct group **result)
{
	int ret;
	int nmemb;
	int fd;

	if (((name == NULL) && (gid == (gid_t)-1)) || (grp == NULL) || (buffer == NULL) ||
	    (result == NULL)) {
		if (result != NULL) {
			*result = NULL;
		}
		return EINVAL;
	}

	if (bufsize < 2) {
		return ERANGE;
	}

	fd = zvfs_open("/etc/group", ZVFS_O_RDONLY, 0);
	if (fd < 0) {
		return EIO;
	}

	while (read_line(fd, buffer, bufsize) > 0) {
		char *p = buffer;
		char *q;

		if (*p == '\0') {
			goto close_erange;
		}

		if (*p == '\n') {
			continue;
		}

		/* name */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		if ((name != NULL) && (strcmp(p, name) != 0)) {
			continue;
		}
		grp->gr_name = p;
		p = q + 1;

		/* password */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		grp->gr_passwd = p;
		p = q + 1;

		/* gid */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		if ((name == NULL) && (atoi(p) != gid)) {
			continue;
		}
		grp->gr_gid = atoi(p);
		p = q + 1;

		/* members */
		q = strchr(p, '\n');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';

		/* count members */
		int min_size;

		nmemb = (p == q) ? 0 : 1 + count(p, ',');
		min_size = (q - buffer + 1 + nmemb * sizeof(char *)) + 32;
		if (bufsize < min_size) {
			goto close_erange;
		}

		/* set up member array inside of buffer */
		grp->gr_mem = (char **)(q + 1);
		grp->gr_mem = (char **)ROUND_UP((uintptr_t)grp->gr_mem, 16);
		grp->gr_mem[nmemb] = NULL;

		for (int i = 0; i < nmemb; ++i) {
			char *x = strchr(p, ',');

			grp->gr_mem[i] = p;
			if (x == NULL) {
				break;
			}
			*x = '\0';
			p = x + 1;
		}

		/* group found \o/ */
		*result = grp;
		ret = 0;
		goto close_ret;
	}

	/* group not found :( )*/
	ret = 0;
	*result = NULL;
	goto close_ret;

close_erange:
	ret = ERANGE;

close_ret:
	zvfs_close(fd);
	return ret;
}

int z_getpw_r(const char *name, uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize,
	      struct passwd **result)
{
	int ret;
	int fd;

	if (((name == NULL) && (uid == (uid_t)-1)) || (pwd == NULL) || (buffer == NULL) ||
	    (result == NULL)) {
		if (result != NULL) {
			*result = NULL;
		}
		return EINVAL;
	}

	if (bufsize < 2) {
		return ERANGE;
	}

	fd = zvfs_open("/etc/passwd", ZVFS_O_RDONLY, 0);
	if (fd < 0) {
		return EIO;
	}

	while (read_line(fd, buffer, bufsize) > 0) {
		char *p = buffer;
		char *q;

		if (*p == '\0') {
			goto close_erange;
		}

		if (*p == '\n') {
			continue;
		}

		/* name */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		if ((name != NULL) && (strcmp(p, name) != 0)) {
			continue;
		}
		pwd->pw_name = p;
		p = q + 1;

		/* password */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		pwd->pw_passwd = p;
		p = q + 1;

		/* uid */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		if ((name == NULL) && (atoi(p) != uid)) {
			continue;
		}
		pwd->pw_uid = atoi(p);
		p = q + 1;

		/* gid */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		pwd->pw_gid = atoi(p);
		p = q + 1;

		/* gecos */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		pwd->pw_gecos = p;
		pwd->pw_comment = NULL;
		p = q + 1;

		/* dir */
		q = strchr(p, ':');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		pwd->pw_dir = p;
		p = q + 1;

		/* shell */
		q = strchr(p, '\n');
		if (q == NULL) {
			goto close_erange;
		}
		*q = '\0';
		pwd->pw_shell = p;

		/* user found \o/ */
		*result = pwd;
		ret = 0;
		goto close_ret;
	}

	/* user not found :( )*/
	ret = 0;
	*result = NULL;
	goto close_ret;

close_erange:
	ret = ERANGE;

close_ret:
	zvfs_close(fd);
	return ret;
}

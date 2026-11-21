/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <mntent.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/sys/zvfs_fs.h>

/* the FILE of <mntent.h> is a cursor on the kernel's mount table, not a stdio stream */
struct mnt_stream {
	int index;
	struct zvfs_mntent ent;
	struct mntent me;
	char opts[sizeof("rw")];
};

FILE *setmntent(const char *filename, const char *type)
{
	struct mnt_stream *s;

	if ((filename == NULL) || (type == NULL) || (*type == '\0')) {
		errno = EINVAL;
		return NULL;
	}
	if (*type != 'r') {
		/* the kernel's table is not written through this interface */
		errno = EROFS;
		return NULL;
	}
	s = calloc(1, sizeof(*s));
	if (s == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	return (FILE *)s;
}

struct mntent *getmntent(FILE *stream)
{
	struct mnt_stream *s = (struct mnt_stream *)stream;

	if (s == NULL) {
		errno = EINVAL;
		return NULL;
	}
	if (zvfs_readmount(&s->index, &s->ent) < 0) {
		return NULL;
	}
	strcpy(s->opts, ((s->ent.mnt_flags & ZVFS_MNT_RDONLY) != 0) ? "ro" : "rw");
	s->me = (struct mntent){
		.mnt_fsname = s->ent.mnt_type,
		.mnt_dir = s->ent.mnt_dir,
		.mnt_type = s->ent.mnt_type,
		.mnt_opts = s->opts,
	};

	return &s->me;
}

int endmntent(FILE *stream)
{
	free(stream);

	return 1;
}

char *hasmntopt(const struct mntent *mnt, const char *opt)
{
	size_t len;
	char *p;

	if ((mnt == NULL) || (mnt->mnt_opts == NULL) || (opt == NULL)) {
		return NULL;
	}
	len = strlen(opt);
	for (p = mnt->mnt_opts; *p != '\0';) {
		size_t n = strcspn(p, ",");

		if ((strncmp(p, opt, len) == 0) && ((n == len) || ((n > len) && (p[len] == '=')))) {
			return p;
		}
		p += n;
		if (*p == ',') {
			p++;
		}
	}

	return NULL;
}

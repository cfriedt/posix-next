/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

#include <zephyr/sys/util.h>

#define SCANDIR_MIN_ENTRIES 8

static void scandir_free(struct dirent **list, size_t count)
{
	while (count > 0) {
		free(list[--count]);
	}
	free(list);
}

int scandir(const char *dir, struct dirent ***namelist, int (*sel)(const struct dirent *),
	    int (*compar)(const struct dirent **, const struct dirent **))
{
	struct dirent **list = NULL;
	size_t count = 0;
	size_t cap = 0;
	DIR *dirp;
	int err;

	dirp = opendir(dir);
	if (dirp == NULL) {
		return -1;
	}

	for (;;) {
		struct dirent *ent;
		struct dirent *copy;

		errno = 0;
		ent = readdir(dirp);
		if (ent == NULL) {
			if (errno != 0) {
				err = errno;
				goto fail;
			}
			break;
		}

		if ((sel != NULL) && (sel(ent) == 0)) {
			continue;
		}

		if (count == cap) {
			size_t newcap = MAX(2 * cap, SCANDIR_MIN_ENTRIES);
			struct dirent **grown = realloc(list, newcap * sizeof(*list));

			if (grown == NULL) {
				err = ENOMEM;
				goto fail;
			}
			list = grown;
			cap = newcap;
		}

		copy = malloc(sizeof(*copy));
		if (copy == NULL) {
			err = ENOMEM;
			goto fail;
		}
		*copy = *ent;
		list[count++] = copy;
	}

	(void)closedir(dirp);

	if (compar != NULL) {
		qsort(list, count, sizeof(*list), (int (*)(const void *, const void *))compar);
	}

	*namelist = list;

	return (int)count;

fail:
	scandir_free(list, count);
	(void)closedir(dirp);
	errno = err;

	return -1;
}

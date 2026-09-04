/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <zephyr/sys/util.h>

#define GETDELIM_MIN_SIZE 64

ssize_t getdelim(char **lineptr, size_t *n, int delimiter, FILE *stream)
{
	char *buf;
	size_t size;
	size_t len = 0;
	ssize_t ret = -1;

	if ((lineptr == NULL) || (n == NULL) || (stream == NULL)) {
		errno = EINVAL;
		return -1;
	}

	/* a null *lineptr requests allocation; only then is *n meaningful */
	buf = *lineptr;
	size = (buf == NULL) ? 0 : *n;

	for (;;) {
		int c = fgetc(stream);

		if (c == EOF) {
			if ((len > 0) && !ferror(stream)) {
				ret = (ssize_t)len;
			}
			break;
		}

		if ((len + 2) > size) {
			size_t newsize = MAX(2 * size, GETDELIM_MIN_SIZE);
			char *newbuf = realloc(buf, newsize);

			if (newbuf == NULL) {
				errno = ENOMEM;
				break;
			}
			buf = newbuf;
			size = newsize;
		}

		buf[len++] = (char)c;

		if (c == delimiter) {
			ret = (ssize_t)len;
			break;
		}
	}

	if ((buf != NULL) && (len < size)) {
		buf[len] = '\0';
	}

	*lineptr = buf;
	*n = size;

	return ret;
}

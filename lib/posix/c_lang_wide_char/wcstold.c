/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

/*
 * Where long double is wider than double, the prebuilt Picolibc declares
 * wcstold() but does not ship it; elsewhere it is an alias of wcstod().
 */
#if __LDBL_MANT_DIG__ != __DBL_MANT_DIG__
long double wcstold(const wchar_t *nptr, wchar_t **endptr)
{
	long double value;
	const wchar_t *start = nptr;
	size_t len;
	char *buf;
	char *end;

	while (iswspace(*start)) {
		start++;
	}

	/* a floating constant is ASCII, so strtold() can parse the ASCII prefix */
	for (len = 0; (start[len] != L'\0') && (start[len] < 0x80); len++) {
	}

	buf = malloc(len + 1);
	if (buf == NULL) {
		if (endptr != NULL) {
			*endptr = (wchar_t *)nptr;
		}
		errno = ENOMEM;
		return 0;
	}
	for (size_t i = 0; i < len; i++) {
		buf[i] = (char)start[i];
	}
	buf[len] = '\0';

	value = strtold(buf, &end);
	if (endptr != NULL) {
		*endptr = (wchar_t *)((end == buf) ? nptr : start + (end - buf));
	}
	free(buf);

	return value;
}
#endif /* __LDBL_MANT_DIG__ != __DBL_MANT_DIG__ */

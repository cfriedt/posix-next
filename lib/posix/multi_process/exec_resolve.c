/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "multi_process_internal.h"

const char *z_posix_exec_resolve(const char *file, char *buf, size_t buflen)
{
	/*
	 * A name containing a slash is used as-is. A bare name is resolved
	 * against the configured default prefix (a stand-in for PATH until
	 * an environment and a filesystem of images exist).
	 */
	if ((file == NULL) || (strchr(file, '/') != NULL)) {
		return file;
	}

	if (snprintf(buf, buflen, "%s%s", CONFIG_POSIX_EXEC_PATH_PREFIX, file) >= (int)buflen) {
		return file;
	}

	return buf;
}

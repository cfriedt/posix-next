/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/zvfs_fs.h>

/* portable filename set */
static const char name_chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* xorshift32: a name's absence is checked, the sequence only has to vary */
static uint32_t tempnam_rand(uint32_t *state)
{
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;
	return *state;
}

static bool tempnam_dir_ok(const char *dir)
{
	return (dir != NULL) && (dir[0] != '\0') &&
	       (zvfs_access(dir, ZVFS_W_OK | ZVFS_X_OK) == 0);
}

char *tempnam(const char *dir, const char *pfx)
{
	const char *candidates[] = {NULL, dir, P_tmpdir, "/tmp"};
	const char *d = NULL;
	uint32_t state;
	size_t dlen;
	size_t plen;
	char *name;

	/* the minimal C library has an environment only through the POSIX layer */
#if !defined(CONFIG_MINIMAL_LIBC) || defined(CONFIG_POSIX_SINGLE_PROCESS) || \
	defined(CONFIG_XSI_SINGLE_PROCESS)
	candidates[0] = getenv("TMPDIR");
#endif
	ARRAY_FOR_EACH(candidates, i) {
		if (tempnam_dir_ok(candidates[i])) {
			d = candidates[i];
			break;
		}
	}
	if (d == NULL) {
		errno = ENOENT;
		return NULL;
	}

	/* at most five characters of the prefix are used */
	plen = (pfx == NULL) ? 0 : MIN(strlen(pfx), 5);
	dlen = strlen(d);
	name = malloc(dlen + 1 + plen + 6 + 1);
	if (name == NULL) {
		errno = ENOMEM;
		return NULL;
	}

	/* two names in use at once come from different buffers, so they differ within a tick */
	state = ((uint32_t)k_uptime_ticks() ^ (uint32_t)(uintptr_t)name) | 1U;
	memcpy(name, d, dlen);
	name[dlen] = '/';
	memcpy(name + dlen + 1, pfx, plen);
	name[dlen + 1 + plen + 6] = '\0';

	for (int attempt = 0; attempt < TMP_MAX; attempt++) {
		char *suffix = name + dlen + 1 + plen;

		for (int i = 0; i < 6; i++) {
			suffix[i] = name_chars[tempnam_rand(&state) % (sizeof(name_chars) - 1)];
		}
		if ((zvfs_access(name, ZVFS_F_OK) < 0) && (errno == ENOENT)) {
			return name;
		}
	}

	free(name);
	errno = EEXIST;
	return NULL;
}

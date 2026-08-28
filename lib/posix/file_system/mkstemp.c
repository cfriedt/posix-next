/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs_fs.h>

/* portable filename set, per XSI mkstemp() */
static const char tmpl_chars[] =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/*
 * A non-cryptographic sequence is sufficient: uniqueness comes from O_EXCL and
 * the retry loop. The state is a local so mkstemp() touches no kernel memory
 * and works unchanged from user mode.
 */
static uint32_t mkstemp_rand(uint32_t *state)
{
	/* xorshift32 */
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;

	return *state;
}

int mkstemp(char *template)
{
	size_t len = strlen(template);
	uint32_t state = (uint32_t)k_uptime_ticks() | 1U;
	char *suffix;

	if (len < 6 || strcmp(&template[len - 6], "XXXXXX") != 0) {
		errno = EINVAL;
		return -1;
	}

	suffix = &template[len - 6];

	for (int attempt = 0; attempt < TMP_MAX; attempt++) {
		int fd;

		for (int i = 0; i < 6; i++) {
			suffix[i] = tmpl_chars[mkstemp_rand(&state) % (sizeof(tmpl_chars) - 1)];
		}

		fd = zvfs_open(template, ZVFS_O_RDWR | ZVFS_O_CREAT | ZVFS_O_EXCL, 0600);
		if (fd >= 0) {
			return fd;
		}

		if (errno != EEXIST) {
			return -1;
		}
	}

	errno = EEXIST;
	return -1;
}

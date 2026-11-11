/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <zephyr/kernel.h>

/* portable filename set, as for mkstemp() */
static const char tmpl_chars[] =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/*
 * A non-cryptographic sequence is sufficient: uniqueness comes from mkdir()
 * failing with EEXIST and the retry loop. The state is a local so mkdtemp()
 * touches no kernel memory and works unchanged from user mode.
 */
static uint32_t mkdtemp_rand(uint32_t *state)
{
	/* xorshift32 */
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;

	return *state;
}

char *mkdtemp(char *template)
{
	size_t len = strlen(template);
	uint32_t state = (uint32_t)k_uptime_ticks() | 1U;
	char *suffix;

	if ((len < 6) || (strcmp(&template[len - 6], "XXXXXX") != 0)) {
		errno = EINVAL;
		return NULL;
	}

	suffix = &template[len - 6];

	for (int attempt = 0; attempt < TMP_MAX; attempt++) {
		for (int i = 0; i < 6; i++) {
			suffix[i] = tmpl_chars[mkdtemp_rand(&state) % (sizeof(tmpl_chars) - 1)];
		}

		if (mkdir(template, 0700) == 0) {
			return template;
		}

		if (errno != EEXIST) {
			return NULL;
		}
	}

	errno = EEXIST;
	return NULL;
}

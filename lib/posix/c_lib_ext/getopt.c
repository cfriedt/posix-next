/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include <zephyr/getopt.h>
#include <zephyr/sys/util_macro.h>

int getopt(int argc, char *const argv[], const char *optstring)
{
	if (!IS_ENABLED(CONFIG_ZEPHYR_GETOPT)) {
		return -1;
	}

	return zephyr_getopt(argc, argv, optstring, NULL, NULL, false, &optarg, &opterr, &optind,
			     &optopt);
}

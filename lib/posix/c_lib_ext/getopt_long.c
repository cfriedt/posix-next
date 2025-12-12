/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>

#include <zephyr/getopt.h>
#include <zephyr/sys/util_macro.h>

int getopt_long(int argc, char *const argv[], const char *optstring, const struct option *longopts,
		int *longindex)
{
	if (!IS_ENABLED(CONFIG_ZEPHYR_GETOPT)) {
		return -1;
	}

	return zephyr_getopt(argc, argv, optstring, longopts, longindex, false, &optarg, &opterr,
			     &optind, &optopt);
}

int getopt_long_only(int argc, char *const argv[], const char *optstring,
		     const struct option *longopts, int *longindex)
{
	if (!IS_ENABLED(CONFIG_ZEPHYR_GETOPT)) {
		return -1;
	}

	return zephyr_getopt(argc, argv, optstring, longopts, longindex, true, &optarg, &opterr,
			     &optind, &optopt);
}

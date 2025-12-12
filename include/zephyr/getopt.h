/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_GETOPT_H_
#define ZEPHYR_GETOPT_H_

#include <stdbool.h>
#include <stddef.h>

#include <zephyr/sys/util_macro.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* struct option is defined in the libc's getopt.h header */
struct option;

int zephyr_getopt(int argc, char *const argv[], const char *optstring,
		  const struct option *longopts, int *longindex, bool longonly, char **optarg,
		  int *opterr, int *optind, int *optopt);

static inline int getopt_r(int argc, char *const argv[], const char *optstring, char **optarg,
			   int *opterr, int *optind, int *optopt)
{
	if (!IS_ENABLED(CONFIG_ZEPHYR_GETOPT)) {
		return -1;
	}

	return zephyr_getopt(argc, argv, NULL, NULL, false, optstring, optarg, opterr, optind,
			     optopt);
}

static inline int getopt_long_r(int argc, char *const argv[], const char *optstring,
				const struct option *longopts, int *longindex, char **optarg,
				int *opterr, int *optind, int *optopt)
{
	if (!IS_ENABLED(CONFIG_ZEPHYR_GETOPT)) {
		return -1;
	}

	return zephyr_getopt(argc, argv, optstring, longopts, longindex, false, optarg, opterr,
			     optind, optopt);
}

static inline int getopt_long_only_r(int argc, char *const argv[], const char *optstring,
				     const struct option *longopts, int *longindex, char **optarg,
				     int *opterr, int *optind, int *optopt)
{
	if (!IS_ENABLED(CONFIG_ZEPHYR_GETOPT)) {
		return -1;
	}

	return zephyr_getopt(argc, argv, optstring, longopts, longindex, true, optarg, opterr,
			     optind, optopt);
}

#if defined(__cplusplus)
}
#endif

#endif

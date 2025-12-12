/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_GETOPT_H_

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

/* struct option is defined in the libc's getopt.h header */
struct option;

int zephyr_getopt(int argc, char *const argv[], const char *optstring,
		  const struct option *longopts, int *longindex, bool longonly, char **optarg,
		  int *opterr, int *optind, int *optopt);

#if defined(__cplusplus)
}
#endif

#endif

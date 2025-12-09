/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

char *optarg;
int opterr, optind, optopt;

int getopt_r(int argc, char *const argv[], const char *optstring, char **optarg, int *opterr,
	     int *optind, int *optopt);

int getopt(int argc, char *const argv[], const char *optstring)
{
	return getopt_r(argc, argv, optstring, &optarg, &opterr, &optind, &optopt);
}

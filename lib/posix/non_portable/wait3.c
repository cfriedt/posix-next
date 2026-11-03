/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/resource.h>
#include <sys/wait.h>

pid_t wait3(int *stat_loc, int options, struct rusage *rusage)
{
	return wait4(-1, stat_loc, options, rusage);
}

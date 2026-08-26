/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/wait.h>

pid_t wait(int *stat_loc)
{
	return waitpid(-1, stat_loc, 0);
}

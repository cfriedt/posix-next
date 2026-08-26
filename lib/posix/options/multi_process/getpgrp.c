/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <unistd.h>

#include <zephyr/sys/process.h>

pid_t getpgrp(void)
{
#ifdef CONFIG_PROCESS
	return (pid_t)sys_pgrp_id(k_getpgid(NULL));
#else
	return z_posix_this_pid();
#endif
}

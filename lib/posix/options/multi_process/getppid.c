/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

pid_t getppid(void)
{
	return (pid_t)sys_process_id(k_getppid());
}

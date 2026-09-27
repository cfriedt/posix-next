/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

pid_t setsid(void)
{
	k_session_t session = sys_setsid();

	if (session == NULL) {
		errno = EPERM;
		return -1;
	}

	return (pid_t)sys_session_id(session);
}

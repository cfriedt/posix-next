/*
 * Copyright (c) 2023 Meta
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <time.h>

int pthread_condattr_setclock(pthread_condattr_t *att, clockid_t clock_id)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if (clock_id != CLOCK_REALTIME && clock_id != CLOCK_MONOTONIC) {
		return -EINVAL;
	}

	if ((attr == NULL) || !attr->initialized) {
		return EINVAL;
	}

	attr->clock = clock_id;

	return 0;
}

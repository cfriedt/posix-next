/*
 * Copyright (c) 2023 Meta
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <time.h>

#include <zephyr/toolchain.h>

int pthread_condattr_getclock(const pthread_condattr_t *ZRESTRICT att,
			      clockid_t *ZRESTRICT clock_id)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if ((attr == NULL) || !attr->initialized) {
		return EINVAL;
	}

	*clock_id = attr->clock;

	return 0;
}

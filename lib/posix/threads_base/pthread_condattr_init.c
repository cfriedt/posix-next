/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>
#include <time.h>

int pthread_condattr_init(pthread_condattr_t *att)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if ((att == NULL) || attr->initialized) {
		return EINVAL;
	}

	attr->clock = CLOCK_REALTIME;
	attr->initialized = true;

	return 0;
}

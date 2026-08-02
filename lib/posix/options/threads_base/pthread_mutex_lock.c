/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>

int pthread_mutex_lock(pthread_mutex_t *m)
{
	return pthread_mutex_lock_common(m, K_FOREVER);
}

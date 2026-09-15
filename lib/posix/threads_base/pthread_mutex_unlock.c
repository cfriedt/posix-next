/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_mutex_unlock(pthread_mutex_t *mu)
{
	return -sys_mutex_unlock(mu);
}

/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>

int pthread_cond_signal(pthread_cond_t *cvar)
{
	return -sys_condvar_signal(cvar);
}

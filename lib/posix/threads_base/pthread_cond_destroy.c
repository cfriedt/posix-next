/*
 * Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "threads_base_internal.h"

#include <errno.h>
#include <pthread.h>

#include <zephyr/sys/thread.h>

int pthread_cond_destroy(pthread_cond_t *cvar)
{
	return -sys_condvar_destroy(cvar);
}

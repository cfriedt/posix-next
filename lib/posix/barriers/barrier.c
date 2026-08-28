/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/sem.h>
#include <zephyr/sys/thread.h>

struct posix_barrier {
	struct k_mutex *mutex;
	struct k_condvar *cond;
	uint32_t max;
	uint32_t count;
};

struct posix_barrierattr {
	uint32_t pshared;
};
BUILD_ASSERT(sizeof(struct posix_barrierattr) <= sizeof(pthread_barrierattr_t));
BUILD_ASSERT(__alignof__(struct posix_barrierattr) <= __alignof__(pthread_barrierattr_t));

static SYS_SEM_DEFINE(posix_barrier_lock, 1, 1);
static struct posix_barrier posix_barrier_pool_storage[CONFIG_MAX_PTHREAD_BARRIER_COUNT];
SYS_ELASTIPOOL_DEFINE_ADVANCED(posix_barrier_pool, sizeof(struct posix_barrier),
			       __alignof(struct posix_barrier), CONFIG_MAX_PTHREAD_BARRIER_COUNT,
			       CONFIG_MAX_PTHREAD_BARRIER_COUNT, NULL, posix_barrier_pool_storage,
			       static);

int pthread_barrier_wait(pthread_barrier_t *b)
{
	int ret;
	int err;
	struct posix_barrier *bar;

	bar = posix_get_pool_obj(&posix_barrier_pool, &posix_barrier_lock, *b);
	if (bar == NULL) {
		return EINVAL;
	}

	err = k_mutex_lock(bar->mutex, K_FOREVER);
	__ASSERT_NO_MSG(err == 0);

	++bar->count;

	if (bar->count == bar->max) {
		bar->count = 0;
		ret = PTHREAD_BARRIER_SERIAL_THREAD;

		goto unlock;
	}

	while (bar->count != 0) {
		err = k_condvar_wait(bar->cond, bar->mutex, K_FOREVER);
		__ASSERT_NO_MSG(err == 0);
		/* Note: count is reset to zero by the serialized thread */
	}

	ret = 0;

unlock:
	err = k_condvar_signal(bar->cond);
	__ASSERT_NO_MSG(err == 0);
	err = k_mutex_unlock(bar->mutex);
	__ASSERT_NO_MSG(err == 0);

	return ret;
}

int pthread_barrier_init(pthread_barrier_t *b, const pthread_barrierattr_t *attr,
			 unsigned int count)
{
	int err = -EAGAIN;
	struct posix_barrier *bar = NULL;

	if (count == 0) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&posix_barrier_lock) {
		err = sys_elastipool_alloc(&posix_barrier_pool, (void **)&bar);
	}
	if (err < 0) {
		return ENOMEM;
	}

	err = sys_mutex_alloc(&bar->mutex, K_MUTEX_NORMAL);
	if (err < 0) {
		goto free_bar;
	}

	err = sys_condvar_alloc(&bar->cond, SYS_CLOCK_REALTIME);
	if (err < 0) {
		(void)sys_mutex_destroy(bar->mutex);
		goto free_bar;
	}

	bar->max = count;
	bar->count = 0;

	*b = (pthread_barrier_t)(uintptr_t)bar;

	return 0;

free_bar:
	SYS_SEM_LOCK(&posix_barrier_lock) {
		(void)sys_elastipool_free(&posix_barrier_pool, (void *)bar);
	}

	return ENOMEM;
}

int pthread_barrier_destroy(pthread_barrier_t *b)
{
	int ret = EINVAL;
	struct posix_barrier *bar;

	bar = posix_get_pool_obj(&posix_barrier_pool, &posix_barrier_lock, *b);
	if (bar == NULL) {
		return EINVAL;
	}

	(void)sys_condvar_destroy(bar->cond);
	(void)sys_mutex_destroy(bar->mutex);

	SYS_SEM_LOCK(&posix_barrier_lock) {
		ret = sys_elastipool_free(&posix_barrier_pool, (void *)bar);
		if (ret < 0) {
			ret = -ret;
		}
	}

	if (ret == 0) {
		*b = -1;
	}

	return ret;
}

int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
	__ASSERT_NO_MSG(attr != NULL);
	struct posix_barrierattr *_attr = (struct posix_barrierattr *)attr;

	_attr->pshared = PTHREAD_PROCESS_PRIVATE;

	return 0;
}

#if defined(_POSIX_THREAD_PROCESS_SHARED)
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
	__ASSERT_NO_MSG(attr != NULL);
	struct posix_barrierattr *_attr = (struct posix_barrierattr *)attr;

	if (pshared != PTHREAD_PROCESS_PRIVATE && pshared != PTHREAD_PROCESS_SHARED) {
		return -EINVAL;
	}

	_attr->pshared = pshared;
	return 0;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *restrict attr,
				   int *restrict pshared)
{
	__ASSERT_NO_MSG(attr != NULL);
	const struct posix_barrierattr *_attr = (const struct posix_barrierattr *)attr;

	*pshared = _attr->pshared;

	return 0;
}
#endif

int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
	ARG_UNUSED(attr);

	return 0;
}

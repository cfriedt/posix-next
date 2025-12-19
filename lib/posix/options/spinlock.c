/*
 * Copyright (c) 2023, Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/sem.h>

static SYS_SEM_DEFINE(posix_spin_lock, 1, 1);
SYS_ELASTIPOOL_DEFINE_STATIC(posix_spin_pool, sizeof(atomic_t), __alignof(atomic_t),
			     CONFIG_MAX_PTHREAD_SPINLOCK_COUNT, CONFIG_MAX_PTHREAD_SPINLOCK_COUNT);

int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	int ret = EINVAL;

	if (lock == NULL ||
	    !(pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED)) {
		/* not specified as part of POSIX but this is the Linux behavior */
		return EINVAL;
	}

	SYS_SEM_LOCK(&posix_spin_lock) {
		uintptr_t l = 0;

		ret = sys_elastipool_alloc(&posix_spin_pool, (void **)&l);
		if (ret < 0) {
			ret = ENOMEM;
		} else {
			atomic_set((atomic_t *)l, 0);
			*lock = (pthread_spinlock_t)(uintptr_t)l;
		}
	}

	return ret;
}

int pthread_spin_destroy(pthread_spinlock_t *lock)
{
	atomic_t *l;
	int ret = EINVAL;

	if (lock == NULL) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&posix_spin_lock) {
		l = posix_get_pool_obj_unlocked(&posix_spin_pool, *lock);
		if (l == NULL) {
			/* not specified as part of POSIX but this is the Linux behavior */
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		ret = sys_elastipool_free(&posix_spin_pool, (void *)l);
		__ASSERT_NO_MSG(ret == 0);
	}

	return ret;
}

static int pthread_spin_lock_common(pthread_spinlock_t *lock, bool wait)
{
	atomic_t *l;
	int ret = EINVAL;

	if (lock == NULL) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&posix_spin_lock) {
		l = posix_get_pool_obj_unlocked(&posix_spin_pool, *lock);
		if (l == NULL) {
			/* not specified as part of POSIX but this is the Linux behavior */
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
	}

	if (ret == 0) {
		while (!atomic_cas(l, 0, 1)) {
			arch_nop();
			if (wait) {
				continue;
			}
			ret = EBUSY;
			break;
		}
	}

	return ret;
}

int pthread_spin_lock(pthread_spinlock_t *lock)
{
	return pthread_spin_lock_common(lock, true);
}

int pthread_spin_trylock(pthread_spinlock_t *lock)
{
	return pthread_spin_lock_common(lock, false);
}

int pthread_spin_unlock(pthread_spinlock_t *lock)
{
	atomic_t *l;
	int ret = EINVAL;

	if (lock == NULL) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&posix_spin_lock) {
		l = posix_get_pool_obj_unlocked(&posix_spin_pool, *lock);
		if (l == NULL) {
			/* not specified as part of POSIX but this is the Linux behavior */
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
	}

	if (ret == 0) {
		while (!atomic_cas(l, 1, 0)) {
			arch_nop();
		}
	}

	return ret;
}

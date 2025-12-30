/*
 * Copyright (c) 2023, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/sys/bitarray.h>

static atomic_t posix_spinlock_pool[CONFIG_MAX_PTHREAD_SPINLOCK_COUNT];
SYS_BITARRAY_DEFINE_STATIC(posix_spinlock_bitarray, CONFIG_MAX_PTHREAD_SPINLOCK_COUNT);

/*
 * We reserve the MSB to mark a pthread_spinlock_t as initialized (from the
 * perspective of the application). With a linear space, this means that
 * the theoretical pthread_spinlock_t range is [0,2147483647].
 */
BUILD_ASSERT(CONFIG_MAX_PTHREAD_SPINLOCK_COUNT < PTHREAD_OBJ_MASK_INIT,
	"CONFIG_MAX_PTHREAD_SPINLOCK_COUNT is too high");

static inline size_t posix_spinlock_to_offset(atomic_t *l)
{
	return l - posix_spinlock_pool;
}

static inline size_t to_posix_spinlock_idx(pthread_spinlock_t lock)
{
	return mark_pthread_obj_uninitialized(lock);
}

static atomic_t *get_posix_spinlock(pthread_spinlock_t *lock)
{
	size_t bit;
	int actually_initialized;

	if (lock == NULL) {
		return NULL;
	}

	/* if the provided spinlock does not claim to be initialized, its invalid */
	bit = to_posix_spinlock_idx(*lock);
	if (!is_pthread_obj_initialized(*lock)) {
		return NULL;
	}

	/* Mask off the MSB to get the actual bit index */
	if (sys_bitarray_test_bit(&posix_spinlock_bitarray, bit, &actually_initialized) < 0) {
		return NULL;
	}

	if (actually_initialized == 0) {
		/* The spinlock claims to be initialized but is actually not */
		return NULL;
	}

	return &posix_spinlock_pool[bit];
}

int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	int ret;
	size_t bit;

	if (lock == NULL ||
	    !(pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED)) {
		/* not specified as part of POSIX but this is the Linux behavior */
		return EINVAL;
	}

	ret = sys_bitarray_alloc(&posix_spinlock_bitarray, 1, &bit);
	if (ret < 0) {
		return ENOMEM;
	}


	*lock = mark_pthread_obj_initialized(bit);

	return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock)
{
	int err;
	size_t bit;
	atomic_t *l;

	l = get_posix_spinlock(lock);
	if (l == NULL) {
		/* not specified as part of POSIX but this is the Linux behavior */
		return EINVAL;
	}

	bit = posix_spinlock_to_offset(l);
	err = sys_bitarray_free(&posix_spinlock_bitarray, 1, bit);
	__ASSERT_NO_MSG(err == 0);

	return 0;
}

static int pthread_spin_lock_common(pthread_spinlock_t *lock, bool wait)
{
	atomic_t *l;

	l = get_posix_spinlock(lock);
	if (l == NULL) {
		/* not specified as part of POSIX but this is the Linux behavior */
		return EINVAL;
	}

	while (!atomic_cas(l, 0, 1)) {
		arch_nop();
		if (wait) {
			continue;
		}
		return EBUSY;
	}

	return 0;
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

	l = get_posix_spinlock(lock);
	if (l == NULL) {
		/* not specified as part of POSIX but this is the Linux behavior */
		return EINVAL;
	}

	atomic_cas(l, 1, 0);

	return 0;
}

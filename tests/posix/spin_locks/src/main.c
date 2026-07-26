/*
 * Copyright (c) 2023, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pthread.h>

#include <zephyr/ztest.h>
#include <zephyr/sys/util.h>

#include "../../common/linux_compat_test.h"

ZTEST(posix_spinlocks, test_spin_init_destroy)
{
	pthread_spinlock_t lock;

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_spin_init(NULL, PTHREAD_PROCESS_PRIVATE), EINVAL,
			      "pthread_spin_init() did not return EINVAL with NULL lock pointer");
		zassert_equal(pthread_spin_init(&lock, 42), EINVAL,
			      "pthread_spin_init() did not return EINVAL with invalid pshared");
		zassert_equal(pthread_spin_destroy(NULL), EINVAL,
			      "pthread_spin_destroy() did not return EINVAL with NULL lock pointer");
	})

	zassert_ok(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE), "pthread_spin_init() failed");
	zassert_ok(pthread_spin_destroy(&lock), "pthread_spin_destroy() failed");

	/* a destroyed handle must not alias the next lock allocated from its pool slot */
	IF_NOT_NATIVE_LIBC({
		pthread_spinlock_t other;

		zassert_ok(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE));
		zassert_ok(pthread_spin_destroy(&lock));
		zassert_ok(pthread_spin_init(&other, PTHREAD_PROCESS_PRIVATE));
		zassert_equal(pthread_spin_lock(&lock), EINVAL,
			      "stale destroyed handle locked the reallocated slot");
		zassert_equal(pthread_spin_trylock(&lock), EINVAL,
			      "stale destroyed handle trylocked the reallocated slot");
		zassert_equal(pthread_spin_unlock(&lock), EINVAL,
			      "stale destroyed handle unlocked the reallocated slot");
		zassert_equal(pthread_spin_destroy(&lock), EINVAL,
			      "stale destroyed handle destroyed the reallocated slot");
		zassert_ok(pthread_spin_destroy(&other));
	})
}

ZTEST(posix_spinlocks, test_spin_descriptor_leak)
{
	pthread_spinlock_t lock[CONFIG_MAX_PTHREAD_SPINLOCK_COUNT];

	/* the host libc does not allocate spin locks from a fixed-size pool */
	posix_test_skip_if_native_libc();

	for (size_t j = 0; j < 2; ++j) {
		for (size_t i = 0; i < ARRAY_SIZE(lock); ++i) {
			zassert_ok(pthread_spin_init(&lock[i], PTHREAD_PROCESS_PRIVATE),
				   "failed to initialize spinlock %zu (rep %zu)", i, j);
		}

		zassert_equal(pthread_spin_init(&lock[CONFIG_MAX_PTHREAD_SPINLOCK_COUNT],
						PTHREAD_PROCESS_PRIVATE),
			      ENOMEM,
			      "should not be able to initialize more than "
			      "CONFIG_MAX_PTHREAD_SPINLOCK_COUNT spinlocks");

		for (size_t i = 0; i < ARRAY_SIZE(lock); ++i) {
			zassert_ok(pthread_spin_destroy(&lock[i]),
				   "failed to destroy spinlock %zu (rep %zu)", i, j);
		}
	}
}

ZTEST(posix_spinlocks, test_spin_lock_unlock)
{
	pthread_spinlock_t lock;

	/* degenerate cases */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(pthread_spin_lock(NULL), EINVAL,
			      "pthread_spin_lock() did not return EINVAL with NULL lock pointer");
		zassert_equal(pthread_spin_trylock(NULL), EINVAL,
			      "pthread_spin_lock() did not return EINVAL with NULL lock pointer");
		zassert_equal(pthread_spin_unlock(NULL), EINVAL,
			      "pthread_spin_lock() did not return EINVAL with NULL lock pointer");
	})

	zassert_ok(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE), "pthread_spin_init() failed");

	zassert_ok(pthread_spin_lock(&lock), "pthread_spin_lock() failed");
	zassert_ok(pthread_spin_unlock(&lock), "pthread_spin_lock() failed");

	zassert_ok(pthread_spin_trylock(&lock), "pthread_spin_trylock() failed");
	zassert_ok(pthread_spin_unlock(&lock), "pthread_spin_unlock() failed");

	zassert_ok(pthread_spin_destroy(&lock), "pthread_spin_destroy() failed");
	IF_NOT_NATIVE_LIBC({
		zassert_equal(lock, (pthread_spinlock_t)-1,
			      "pthread_spin_destroy() did not invalidate the lock handle");
		zassert_equal(pthread_spin_destroy(&lock), EINVAL,
			      "pthread_spin_destroy() on a destroyed lock did not return EINVAL");
		zassert_equal(pthread_spin_lock(&lock), EINVAL,
			      "pthread_spin_lock() on a destroyed lock did not return EINVAL");
		zassert_equal(pthread_spin_trylock(&lock), EINVAL,
			      "pthread_spin_trylock() on a destroyed lock did not return EINVAL");
		zassert_equal(pthread_spin_unlock(&lock), EINVAL,
			      "pthread_spin_unlock() on a destroyed lock did not return EINVAL");
	})
}

ZTEST_SUITE(posix_spinlocks, NULL, NULL, NULL, NULL, NULL);

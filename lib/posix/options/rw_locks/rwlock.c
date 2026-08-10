/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/sem.h>
#include <zephyr/sys/timeutil.h>

#define CONCURRENT_READER_LIMIT  (CONFIG_POSIX_THREAD_THREADS_MAX + 1)

struct posix_rwlock {
	struct sys_sem rd_sem;
	struct sys_sem wr_sem;
	struct sys_sem reader_active; /* blocks WR till reader has acquired lock */
	k_tid_t wr_owner;
};

struct posix_rwlockattr {
	bool initialized: 1;
	bool pshared: 1;
};

static uint32_t read_lock_acquire(struct posix_rwlock *rwl, k_timeout_t timeout);
static uint32_t write_lock_acquire(struct posix_rwlock *rwl, k_timeout_t timeout);

LOG_MODULE_REGISTER(pthread_rwlock, CONFIG_PTHREAD_RWLOCK_LOG_LEVEL);

static SYS_SEM_DEFINE(posix_rwlock_lock, 1, 1);
/* typed (not raw) storage, so the kobject scanner registers each embedded sys_sem futex */
static struct posix_rwlock posix_rwlock_pool_storage[CONFIG_MAX_PTHREAD_RWLOCK_COUNT];
SYS_ELASTIPOOL_DEFINE_ADVANCED(posix_rwlock_pool, sizeof(struct posix_rwlock),
			       __alignof(struct posix_rwlock), CONFIG_MAX_PTHREAD_RWLOCK_COUNT,
			       CONFIG_MAX_PTHREAD_RWLOCK_COUNT, NULL, posix_rwlock_pool_storage,
			       static);

/**
 * @brief Initialize read-write lock object.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_init(pthread_rwlock_t *rwlock,
			const pthread_rwlockattr_t *attr)
{
	struct posix_rwlock *rwl;

	ARG_UNUSED(attr);
	*rwlock = PTHREAD_RWLOCK_INITIALIZER;

	rwl = posix_init_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock, NULL);
	if (rwl == NULL) {
		return ENOMEM;
	}

	sys_sem_init(&rwl->rd_sem, CONCURRENT_READER_LIMIT, CONCURRENT_READER_LIMIT);
	sys_sem_init(&rwl->wr_sem, 1, 1);
	sys_sem_init(&rwl->reader_active, 1, 1);
	rwl->wr_owner = NULL;

	*rwlock = (pthread_rwlock_t)(uintptr_t)rwl;

	LOG_DBG("Initialized rwlock %p", rwl);

	return 0;
}

/**
 * @brief Destroy read-write lock object.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
	int err;
	int ret = EINVAL;
	struct posix_rwlock *rwl;

	SYS_SEM_LOCK(&posix_rwlock_lock) {
		rwl = posix_get_pool_obj_unlocked(&posix_rwlock_pool, *rwlock);
		if (rwl == NULL) {
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		if (rwl->wr_owner != NULL) {
			ret = EBUSY;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
		err = sys_elastipool_free(&posix_rwlock_pool, (void *)rwl);
		__ASSERT_NO_MSG(err == 0);
	}

	return ret;
}

/**
 * @brief Lock a read-write lock object for reading.
 *
 * API behaviour is unpredictable if number of concurrent reader
 * lock held is greater than CONCURRENT_READER_LIMIT.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
	struct posix_rwlock *rwl;

	rwl = posix_get_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock);
	if (rwl == NULL) {
		return EINVAL;
	}

	return read_lock_acquire(rwl, K_FOREVER);
}

/**
 * @brief Lock a read-write lock object for reading within specific time.
 *
 * API behaviour is unpredictable if number of concurrent reader
 * lock held is greater than CONCURRENT_READER_LIMIT.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock,
			       const struct timespec *abstime)
{
	uint32_t ret = 0U;
	struct posix_rwlock *rwl;

	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		LOG_DBG("%s is invalid", "abstime");
		return EINVAL;
	}

	rwl = posix_get_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock);
	if (rwl == NULL) {
		return EINVAL;
	}

	/* TODO(clock-settime-reactive-waits): the CLOCK_REALTIME offset is baked in here */
	if (read_lock_acquire(rwl, timespec_abs_to_timeout(SYS_CLOCK_REALTIME, abstime)) != 0U) {
		ret = ETIMEDOUT;
	}

	return ret;
}

/**
 * @brief Lock a read-write lock object for reading immediately.
 *
 * API behaviour is unpredictable if number of concurrent reader
 * lock held is greater than CONCURRENT_READER_LIMIT.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
	struct posix_rwlock *rwl;

	rwl = posix_get_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock);
	if (rwl == NULL) {
		return EINVAL;
	}

	return read_lock_acquire(rwl, K_NO_WAIT);
}

/**
 * @brief Lock a read-write lock object for writing.
 *
 * Write lock does not have priority over reader lock,
 * threads get lock based on priority.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
	struct posix_rwlock *rwl;

	rwl = posix_get_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock);
	if (rwl == NULL) {
		return EINVAL;
	}

	return write_lock_acquire(rwl, K_FOREVER);
}

/**
 * @brief Lock a read-write lock object for writing within specific time.
 *
 * Write lock does not have priority over reader lock,
 * threads get lock based on priority.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock,
			       const struct timespec *abstime)
{
	uint32_t ret = 0U;
	struct posix_rwlock *rwl;

	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		LOG_DBG("%s is invalid", "abstime");
		return EINVAL;
	}

	rwl = posix_get_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock);
	if (rwl == NULL) {
		return EINVAL;
	}

	/* TODO(clock-settime-reactive-waits): the CLOCK_REALTIME offset is baked in here */
	if (write_lock_acquire(rwl, timespec_abs_to_timeout(SYS_CLOCK_REALTIME, abstime)) != 0U) {
		ret = ETIMEDOUT;
	}

	return ret;
}

/**
 * @brief Lock a read-write lock object for writing immediately.
 *
 * Write lock does not have priority over reader lock,
 * threads get lock based on priority.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
	struct posix_rwlock *rwl;

	rwl = posix_get_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock);
	if (rwl == NULL) {
		return EINVAL;
	}

	return write_lock_acquire(rwl, K_NO_WAIT);
}

/**
 *
 * @brief Unlock a read-write lock object.
 *
 * See IEEE 1003.1
 */
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
	struct posix_rwlock *rwl;

	rwl = posix_get_pool_obj(&posix_rwlock_pool, &posix_rwlock_lock, *rwlock);
	if (rwl == NULL) {
		return EINVAL;
	}

	if (k_current_get() == rwl->wr_owner) {
		/* Write unlock */
		rwl->wr_owner = NULL;
		(void)sys_sem_give(&rwl->reader_active);
		(void)sys_sem_give(&rwl->wr_sem);
	} else {
		/* Read unlock */
		(void)sys_sem_give(&rwl->rd_sem);

		if (sys_sem_count_get(&rwl->rd_sem) == CONCURRENT_READER_LIMIT) {
			/* Last read lock, unlock writer */
			(void)sys_sem_give(&rwl->reader_active);
		}
	}
	return 0;
}

static uint32_t read_lock_acquire(struct posix_rwlock *rwl, k_timeout_t timeout)
{
	uint32_t ret = 0U;

	if (sys_sem_take(&rwl->wr_sem, timeout) == 0) {
		(void)sys_sem_take(&rwl->reader_active, K_NO_WAIT);
		(void)sys_sem_take(&rwl->rd_sem, K_NO_WAIT);
		(void)sys_sem_give(&rwl->wr_sem);
	} else {
		ret = EBUSY;
	}

	return ret;
}

static uint32_t write_lock_acquire(struct posix_rwlock *rwl, k_timeout_t timeout)
{
	uint32_t ret = 0U;
	k_timepoint_t end = sys_timepoint_calc(timeout);

	/* waiting for release of write lock */
	if (sys_sem_take(&rwl->wr_sem, timeout) == 0) {
		/* waiting for reader to complete operation */
		if (sys_sem_take(&rwl->reader_active, sys_timepoint_timeout(end)) == 0) {
			rwl->wr_owner = k_current_get();
		} else {
			(void)sys_sem_give(&rwl->wr_sem);
			ret = EBUSY;
		}

	} else {
		ret = EBUSY;
	}
	return ret;
}

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *ZRESTRICT attr,
				  int *ZRESTRICT pshared)
{
	struct posix_rwlockattr *const a = (struct posix_rwlockattr *)attr;

	if (a == NULL || !a->initialized) {
		return EINVAL;
	}

	*pshared = a->pshared;

	return 0;
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
	struct posix_rwlockattr *const a = (struct posix_rwlockattr *)attr;

	if (a == NULL || !a->initialized) {
		return EINVAL;
	}

	if (!(pshared == PTHREAD_PROCESS_PRIVATE || pshared == PTHREAD_PROCESS_SHARED)) {
		return EINVAL;
	}

	a->pshared = pshared;

	return 0;
}

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
	struct posix_rwlockattr *const a = (struct posix_rwlockattr *)attr;

	if (a == NULL) {
		return EINVAL;
	}

	*a = (struct posix_rwlockattr){
		.initialized = true,
		.pshared = PTHREAD_PROCESS_PRIVATE,
	};

	return 0;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
	struct posix_rwlockattr *const a = (struct posix_rwlockattr *)attr;

	if (a == NULL || !a->initialized) {
		return EINVAL;
	}

	*a = (struct posix_rwlockattr){0};

	return 0;
}

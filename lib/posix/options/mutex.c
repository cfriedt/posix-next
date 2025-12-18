/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2023 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"
#include "posix_internal.h"

#include <pthread.h>
#include <sys/types.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/bitarray.h>
#include <zephyr/sys/sem.h>

LOG_MODULE_REGISTER(pthread_mutex, CONFIG_PTHREAD_MUTEX_LOG_LEVEL);

#define MUTEX_MAX_REC_LOCK 32767

struct pthread_mutexattr {
	unsigned char type: 2;
	bool initialized: 1;
};
BUILD_ASSERT(sizeof(pthread_mutexattr_t) >= sizeof(struct pthread_mutexattr));

/*
 *  Default mutex attrs.
 */
static const struct pthread_mutexattr def_attr = {
	.type = PTHREAD_MUTEX_DEFAULT,
};

static SYS_SEM_DEFINE(posix_mutex_lock, 1, 1);
SYS_ELASTIPOOL_DEFINE_STATIC(posix_mutex_pool, sizeof(struct k_mutex), __alignof(struct k_mutex),
			     CONFIG_MAX_PTHREAD_MUTEX_COUNT, CONFIG_MAX_PTHREAD_MUTEX_COUNT);
/* fixme: this should just be added to zephyr's mutex descriptor */
static __pinned_bss uint8_t posix_mutex_type[CONFIG_MAX_PTHREAD_MUTEX_COUNT];

static inline size_t posix_mutex_to_offset(struct k_mutex *m)
{
	/* fixme: this prevents dynamic growth with elastipool */
	return ((uint8_t *)m - posix_mutex_pool.config->storage) /
	       ROUND_UP(posix_mutex_pool.config->obj_size, posix_mutex_pool.config->obj_align);
}

static void mutex_init_pool_obj_cb(void *obj)
{
	int err;
	struct k_mutex *const m = (struct k_mutex *)obj;

	err = k_mutex_init(m);
	__ASSERT_NO_MSG(err == 0);
}

struct k_mutex *to_posix_mutex(pthread_mutex_t *mu)
{
	return posix_init_pool_obj(&posix_mutex_pool, &posix_mutex_lock, *mu,
				   mutex_init_pool_obj_cb);
}

static int acquire_mutex(pthread_mutex_t *mu, k_timeout_t timeout)
{
	int type = -1;
	size_t bit = -1;
	int ret = EINVAL;
	size_t lock_count = -1;
	struct k_mutex *m = NULL;
	struct k_thread *owner = NULL;

	SYS_SEM_LOCK(&posix_mutex_lock) {
		m = posix_init_pool_obj_unlocked(&posix_mutex_pool, *mu, mutex_init_pool_obj_cb);
		if (m == NULL) {
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}
		*mu = (pthread_mutex_t)(uintptr_t)m;

		ret = 0;
		bit = posix_mutex_to_offset(m);
		type = posix_mutex_type[bit];
		owner = m->owner;
		lock_count = m->lock_count;

		LOG_DBG("Locking mutex %p (bit %zu, type %d) with timeout %" PRIx64, m, bit, type,
			(int64_t)timeout.ticks);
	}

	if (ret != 0) {
		goto handle_error;
	}

	if (owner == k_current_get()) {
		switch (type) {
		case PTHREAD_MUTEX_DEFAULT:
		case PTHREAD_MUTEX_NORMAL:
			if (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
				LOG_DBG("Timeout locking mutex %p", m);
				ret = EBUSY;
				break;
			}
			/* On most POSIX systems, this usually results in an infinite loop */
			LOG_DBG("Attempt to relock non-recursive mutex %p", m);
			do {
				(void)k_sleep(K_FOREVER);
			} while (true);
			CODE_UNREACHABLE;
			break;
		case PTHREAD_MUTEX_RECURSIVE:
			if (lock_count >= MUTEX_MAX_REC_LOCK) {
				LOG_DBG("Mutex %p locked recursively too many times", m);
				ret = EAGAIN;
			}
			break;
		case PTHREAD_MUTEX_ERRORCHECK:
			LOG_DBG("Attempt to recursively lock non-recursive mutex %p", m);
			ret = EDEADLK;
			break;
		default:
			__ASSERT(false, "invalid pthread type %d", type);
			ret = EINVAL;
			break;
		}
	}

	if (ret == 0) {
		ret = k_mutex_lock(m, timeout);
		if (ret == -EAGAIN) {
			LOG_DBG("Timeout locking mutex %p", m);
			/*
			 * special quirk - k_mutex_lock() returns EAGAIN if a timeout occurs, but
			 * for pthreads, that means something different
			 */
			ret = ETIMEDOUT;
		}
	}

handle_error:
	if (ret < 0) {
		LOG_DBG("k_mutex_unlock() failed: %d", ret);
		ret = -ret;
	}

	if (ret == 0) {
		LOG_DBG("Locked mutex %p", m);
	}

	return ret;
}

/**
 * @brief Lock POSIX mutex with non-blocking call.
 *
 * See IEEE 1003.1
 */
int pthread_mutex_trylock(pthread_mutex_t *m)
{
	return acquire_mutex(m, K_NO_WAIT);
}

/**
 * @brief Lock POSIX mutex with timeout.
 *
 *
 * See IEEE 1003.1
 */
int pthread_mutex_timedlock(pthread_mutex_t *m,
			    const struct timespec *abstime)
{
	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		LOG_DBG("%s is invalid", "abstime");
		return EINVAL;
	}

	return acquire_mutex(m, K_MSEC(timespec_to_timeoutms(CLOCK_REALTIME, abstime)));
}

/**
 * @brief Initialize POSIX mutex.
 *
 * See IEEE 1003.1
 */
int pthread_mutex_init(pthread_mutex_t *mu, const pthread_mutexattr_t *_attr)
{
	size_t bit;
	int type = -1;
	int ret = ENOMEM;
	struct k_mutex *m;
	const struct pthread_mutexattr *attr = (const struct pthread_mutexattr *)_attr;

	*mu = PTHREAD_MUTEX_INITIALIZER;

	SYS_SEM_LOCK(&posix_mutex_lock) {
		m = posix_init_pool_obj_unlocked(&posix_mutex_pool, *mu, mutex_init_pool_obj_cb);
		if (m == NULL) {
			ret = ENOMEM;
			SYS_SEM_LOCK_BREAK;
		}
		*mu = (pthread_mutex_t)(uintptr_t)m;

		bit = posix_mutex_to_offset(m);
		if (attr == NULL) {
			posix_mutex_type[bit] = def_attr.type;
		} else {
			posix_mutex_type[bit] = attr->type;
		}

		type = posix_mutex_type[bit];
		ret = 0;
	}

	if (ret == 0) {
		LOG_DBG("Initialized mutex %p, bit %zu, type %d", m, bit, type);
	}

	return ret;
}

/**
 * @brief Lock POSIX mutex with blocking call.
 *
 * See IEEE 1003.1
 */
int pthread_mutex_lock(pthread_mutex_t *m)
{
	return acquire_mutex(m, K_FOREVER);
}

/**
 * @brief Unlock POSIX mutex.
 *
 * See IEEE 1003.1
 */
int pthread_mutex_unlock(pthread_mutex_t *mu)
{
	int ret = EINVAL;
	struct k_mutex *m;

	SYS_SEM_LOCK(&posix_mutex_lock) {
		m = posix_get_pool_obj_unlocked(&posix_mutex_pool, *mu);
		if (m == NULL) {
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		ret = k_mutex_unlock(m);
		if (ret < 0) {
			LOG_DBG("k_mutex_unlock() failed: %d", ret);
			ret = -ret;
			SYS_SEM_LOCK_BREAK;
		}

		ret = 0;
	}

	if (ret == 0) {
		LOG_DBG("Unlocked mutex %p", m);
	}

	return ret;
}

/**
 * @brief Destroy POSIX mutex.
 *
 * See IEEE 1003.1
 */
int pthread_mutex_destroy(pthread_mutex_t *mu)
{
	int ret = EINVAL;
	struct k_mutex *m;

	SYS_SEM_LOCK(&posix_mutex_lock) {
		m = posix_get_pool_obj_unlocked(&posix_mutex_pool, *mu);
		if (m == NULL) {
			ret = EINVAL;
			SYS_SEM_LOCK_BREAK;
		}

		ret = sys_elastipool_free(&posix_mutex_pool, m);
		__ASSERT_NO_MSG(ret == 0);
	}

	if (ret == 0) {
		LOG_DBG("Destroyed mutex %p", m);
	}

	return ret;
}

#if defined(_POSIX_THREAD_PRIO_PROTECT)
/**
 * @brief Read protocol attribute for mutex.
 *
 * See IEEE 1003.1
 */
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr,
				  int *protocol)
{
	if ((attr == NULL) || (protocol == NULL)) {
		return EINVAL;
	}

	*protocol = PTHREAD_PRIO_NONE;
	return 0;
}

/**
 * @brief Set protocol attribute for mutex.
 *
 * See IEEE 1003.1
 */
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
	if (attr == NULL) {
		return EINVAL;
	}

	switch (protocol) {
	case PTHREAD_PRIO_NONE:
		return 0;
	case PTHREAD_PRIO_INHERIT:
		return ENOTSUP;
	case PTHREAD_PRIO_PROTECT:
		return ENOTSUP;
	default:
		return EINVAL;
	}
}
#endif

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (a == NULL) {
		return EINVAL;
	}

	a->type = PTHREAD_MUTEX_DEFAULT;
	a->initialized = true;

	return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (a == NULL || !a->initialized) {
		return EINVAL;
	}

	*a = (struct pthread_mutexattr){0};

	return 0;
}

/**
 * @brief Read type attribute for mutex.
 *
 * See IEEE 1003.1
 */
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
	const struct pthread_mutexattr *a = (const struct pthread_mutexattr *)attr;

	if (a == NULL || type == NULL || !a->initialized) {
		return EINVAL;
	}

	*type = a->type;

	return 0;
}

/**
 * @brief Set type attribute for mutex.
 *
 * See IEEE 1003.1
 */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
	struct pthread_mutexattr *const a = (struct pthread_mutexattr *)attr;

	if (a == NULL || !a->initialized) {
		return EINVAL;
	}

	switch (type) {
	case PTHREAD_MUTEX_NORMAL:
	case PTHREAD_MUTEX_RECURSIVE:
	case PTHREAD_MUTEX_ERRORCHECK:
	case PTHREAD_MUTEX_DEFAULT:
		a->type = type;
		return 0;
	default:
		return EINVAL;
	}
}

#ifdef CONFIG_POSIX_THREAD_PRIO_PROTECT
int pthread_mutex_getprioceiling(const pthread_mutex_t *mutex, int *prioceiling)
{
	ARG_UNUSED(mutex);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}

int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int prioceiling, int *old_ceiling)
{
	ARG_UNUSED(mutex);
	ARG_UNUSED(prioceiling);
	ARG_UNUSED(old_ceiling);

	return ENOSYS;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
	ARG_UNUSED(attr);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
	ARG_UNUSED(attr);
	ARG_UNUSED(prioceiling);

	return ENOSYS;
}

#endif /* CONFIG_POSIX_THREAD_PRIO_PROTECT */

__boot_func
static int pthread_mutex_pool_init(void)
{
	int err;
	size_t i;

	for (i = 0; i < CONFIG_MAX_PTHREAD_MUTEX_COUNT; ++i) {
		struct k_mutex *const m = (struct k_mutex *)posix_mutex_pool.config->storage;

		err = k_mutex_init(m);
		__ASSERT_NO_MSG(err == 0);
		posix_mutex_type[i] = PTHREAD_MUTEX_DEFAULT;
	}

	return 0;
}
SYS_INIT(pthread_mutex_pool_init, PRE_KERNEL_1, 0);

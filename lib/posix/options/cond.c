/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2023 Meta
 * Copyright (c) 2025, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_clock.h"
#include "posix_internal.h"

#include <pthread.h>

#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/sem.h>

BUILD_ASSERT(sizeof(pthread_condattr_t) >= sizeof(struct posix_condattr));
#ifdef CONFIG_SYS_THREAD
BUILD_ASSERT(sizeof(pthread_condattr_t) >= sizeof(((struct k_condvar *)0)->flags));
#endif

LOG_MODULE_REGISTER(pthread_cond, CONFIG_PTHREAD_COND_LOG_LEVEL);

static SYS_SEM_DEFINE(posix_cond_lock, 1, 1);
SYS_ELASTIPOOL_DEFINE_STATIC(posix_cond_pool, sizeof(struct posix_cond),
			     __alignof(struct posix_cond), CONFIG_MAX_PTHREAD_COND_COUNT,
			     CONFIG_MAX_PTHREAD_COND_COUNT);

static void cond_init_pool_obj_cb(void *obj)
{
	struct posix_cond *const cv = (struct posix_cond *)obj;

#if defined(CONFIG_SYS_THREAD)
	(void)pthread_condattr_init((pthread_condattr_t *)&cv->condvar.flags);
#else
	(void)pthread_condattr_init((pthread_condattr_t *)&cv->attr);
#endif
}

static int cond_wait(pthread_cond_t *cvar, pthread_mutex_t *mu, const struct timespec *abstime)
{
	int ret;
	struct k_mutex *m;
	struct posix_cond *cv;
	k_timeout_t timeout = K_FOREVER;

	m = to_posix_mutex(mu);
	cv = posix_init_pool_obj(&posix_cond_pool, &posix_cond_lock, *cvar, cond_init_pool_obj_cb);

	if (cv == NULL) {
		return EINVAL;
	}

	if (m == NULL) {
		if (*cvar == POSIX_OBJ_INITIALIZER) {
			SYS_SEM_LOCK(&posix_cond_lock) {
				sys_elastipool_free(&posix_cond_pool, (void *)cv);
			}
		}
		return EINVAL;
	}

	if (*cvar == POSIX_OBJ_INITIALIZER) {
		*cvar = (pthread_cond_t)(uintptr_t)cv;
	}

	if (*mu == POSIX_OBJ_INITIALIZER) {
		*mu = (pthread_mutex_t)(uintptr_t)m;
	}

	if (abstime != NULL) {
#ifdef CONFIG_SYS_THREAD
		struct posix_condattr *const ap = (struct posix_condattr *)&cv->condvar.flags;
#else
		struct posix_condattr *const ap = (struct posix_condattr *)&cv->attr;
#endif

		timeout = K_MSEC(timespec_to_timeoutms(ap->clock, abstime));
	}

	LOG_DBG("Waiting on cond %p with timeout %" PRIx64, cv, (int64_t)timeout.ticks);
	ret = k_condvar_wait(&cv->condvar, m, timeout);
	if (ret == -EAGAIN) {
		LOG_DBG("Timeout waiting on cond %p", cv);
		ret = ETIMEDOUT;
	} else if (ret < 0) {
		LOG_DBG("k_condvar_wait() failed: %d", ret);
		ret = -ret;
	} else {
		__ASSERT_NO_MSG(ret == 0);
		LOG_DBG("Cond %p received signal", cv);
	}

	return ret;
}

int pthread_cond_signal(pthread_cond_t *cvar)
{
	int ret;
	struct posix_cond *cv;

	cv = posix_init_pool_obj(&posix_cond_pool, &posix_cond_lock, *cvar, cond_init_pool_obj_cb);
	if (cv == NULL) {
		return EINVAL;
	}

	if (*cvar == POSIX_OBJ_INITIALIZER) {
		*cvar = (pthread_cond_t)(uintptr_t)cv;
	}

	LOG_DBG("Signaling cond %p", cv);
	ret = k_condvar_signal(&cv->condvar);
	if (ret < 0) {
		LOG_DBG("k_condvar_signal() failed: %d", ret);
		return -ret;
	}

	__ASSERT_NO_MSG(ret == 0);

	return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cvar)
{
	int ret;
	struct posix_cond *cv;

	cv = posix_init_pool_obj(&posix_cond_pool, &posix_cond_lock, *cvar, cond_init_pool_obj_cb);
	if (cv == NULL) {
		return EINVAL;
	}

	if (*cvar == POSIX_OBJ_INITIALIZER) {
		*cvar = (pthread_cond_t)(uintptr_t)cv;
	}

	LOG_DBG("Broadcasting on cond %p", cv);
	ret = k_condvar_broadcast(&cv->condvar);
	if (ret < 0) {
		LOG_DBG("k_condvar_broadcast() failed: %d", ret);
		return -ret;
	}

	__ASSERT_NO_MSG(ret >= 0);

	return 0;
}

int pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *mut)
{
	return cond_wait(cv, mut, NULL);
}

int pthread_cond_timedwait(pthread_cond_t *cv, pthread_mutex_t *mut, const struct timespec *abstime)
{
	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		LOG_DBG("%s is invalid", "abstime");
		return EINVAL;
	}

	return cond_wait(cv, mut, abstime);
}

int pthread_cond_init(pthread_cond_t *cvar, const pthread_condattr_t *att)
{
	struct posix_cond *cv;
	struct posix_condattr *attr = (struct posix_condattr *)att;

	if (attr != NULL) {
		if (!attr->initialized) {
			return EINVAL;
		}
	}

	*cvar = PTHREAD_COND_INITIALIZER;
	cv = posix_init_pool_obj(&posix_cond_pool, &posix_cond_lock, *cvar, cond_init_pool_obj_cb);
	if (cv == NULL) {
		return EINVAL;
	}

	if (*cvar == PTHREAD_COND_INITIALIZER) {
		*cvar = (pthread_cond_t)(uintptr_t)cv;
	}

	if (attr != NULL) {
		if (!attr->initialized) {
			return EINVAL;
		}

#ifdef CONFIG_SYS_THREAD
		(void)pthread_condattr_destroy((pthread_condattr_t *)&cv->condvar.flags);
		cv->condvar.flags = *((unsigned char *)&attr);
#else
		(void)pthread_condattr_destroy((pthread_condattr_t *)&cv->attr);
		cv->attr = *attr;
#endif
	}

	LOG_DBG("Initialized cond %p", cv);
	*cvar = (pthread_cond_t)(uintptr_t)cv;

	return 0;
}

int pthread_cond_destroy(pthread_cond_t *cvar)
{
	int ret = EINVAL;
	struct posix_cond *cv;

	cv = posix_get_pool_obj(&posix_cond_pool, &posix_cond_lock, *cvar);
	if (cv == NULL) {
		return EINVAL;
	}

	SYS_SEM_LOCK(&posix_cond_lock) {
		ret = sys_elastipool_free(&posix_cond_pool, (void *)cv);
		if (ret < 0) {
			ret = -ret;
		}
	}

	if (ret == 0) {
		LOG_DBG("Destroyed cond %p", cv);
		*cvar = -1;
	}

	return ret;
}

__boot_func
static int pthread_cond_pool_init(void)
{
	int err;
	size_t i;

	for (i = 0; i < CONFIG_MAX_PTHREAD_COND_COUNT; ++i) {
		struct posix_cond *cv = &((struct posix_cond *)posix_cond_pool.config->storage)[i];

		err = k_condvar_init(&cv->condvar);
		__ASSERT_NO_MSG(err == 0);
	}

	return 0;
}

int pthread_condattr_init(pthread_condattr_t *att)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if (att == NULL) {
		return EINVAL;
	}
	if (attr->initialized) {
		LOG_DBG("%s %s initialized", "attribute", "already");
		return EINVAL;
	}

	attr->clock = CLOCK_REALTIME;
	attr->initialized = true;

	return 0;
}

int pthread_condattr_destroy(pthread_condattr_t *att)
{
	struct posix_condattr *const attr = (struct posix_condattr *)att;

	if ((attr == NULL) || !attr->initialized) {
		LOG_DBG("%s %s initialized", "attribute", "not");
		return EINVAL;
	}

	*attr = (struct posix_condattr){0};

	return 0;
}

SYS_INIT(pthread_cond_pool_init, PRE_KERNEL_1, 0);

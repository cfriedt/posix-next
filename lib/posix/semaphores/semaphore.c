/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/sem.h>
#include <zephyr/sys/timeutil.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

struct nsem_obj {
	sys_snode_t snode;
	sem_t sem;
	int ref_count;
	bool linked;
	char name[CONFIG_POSIX_SEM_NAMELEN_MAX];
};

static sys_slist_t nsem_list;

static SYS_SEM_DEFINE(nsem_lock, 1, 1);
/* typed (not raw) storage, so the kobject scanner registers each embedded sem_t futex */
static struct nsem_obj nsem_pool_storage[CONFIG_POSIX_SEM_NSEMS_MAX];
SYS_ELASTIPOOL_DEFINE_ADVANCED(nsem_pool, sizeof(struct nsem_obj), __alignof(struct nsem_obj),
			       CONFIG_POSIX_SEM_NSEMS_MAX, CONFIG_POSIX_SEM_NSEMS_MAX, NULL,
			       nsem_pool_storage, static);

static inline void nsem_list_lock(void)
{
	__unused int ret = sys_sem_take(&nsem_lock, K_FOREVER);

	__ASSERT(ret == 0, "nsem_list_lock() failed: %d", ret);
}

static inline void nsem_list_unlock(void)
{
	(void)sys_sem_give(&nsem_lock);
}

static struct nsem_obj *nsem_find(const char *name)
{
	struct nsem_obj *nsem;

	SYS_SLIST_FOR_EACH_CONTAINER(&nsem_list, nsem, snode) {
		if (nsem->linked && (strcmp(nsem->name, name) == 0)) {
			return nsem;
		}
	}

	return NULL;
}

/* Remove a named semaphore if it isn't used */
static void nsem_unref(struct nsem_obj *nsem)
{
	nsem->ref_count -= 1;
	__ASSERT(nsem->ref_count >= 0, "ref_count may not be negative");

	if (nsem->ref_count == 0) {
		__ASSERT(!nsem->linked, "ref_count is 0 but sem is not unlinked");

		sys_slist_find_and_remove(&nsem_list, (sys_snode_t *) nsem);

		__unused int ret = sys_elastipool_free(&nsem_pool, (void *)nsem);

		__ASSERT(ret == 0, "failed to free named semaphore %p: %d", nsem, ret);
	}
}

/**
 * @brief Destroy semaphore.
 *
 * see IEEE 1003.1
 */
int sem_destroy(sem_t *semaphore)
{
	if (semaphore == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (sys_sem_has_waiters(semaphore)) {
		errno = EBUSY;
		return -1;
	}

	(void)sys_sem_init(semaphore, 0, CONFIG_POSIX_SEM_VALUE_MAX);
	return 0;
}

/**
 * @brief Get value of semaphore.
 *
 * See IEEE 1003.1
 */
int sem_getvalue(sem_t *semaphore, int *value)
{
	if (semaphore == NULL) {
		errno = EINVAL;
		return -1;
	}

	*value = (int)sys_sem_count_get(semaphore);

	return 0;
}
/**
 * @brief Initialize semaphore.
 *
 * See IEEE 1003.1
 */
int sem_init(sem_t *semaphore, int pshared, unsigned int value)
{
	if (value > CONFIG_POSIX_SEM_VALUE_MAX) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Zephyr has no concept of process, so only thread shared
	 * semaphore makes sense in here.
	 */
	__ASSERT(pshared == 0, "pshared should be 0");

	(void)sys_sem_init(semaphore, value, CONFIG_POSIX_SEM_VALUE_MAX);

	return 0;
}

/**
 * @brief Unlock a semaphore.
 *
 * See IEEE 1003.1
 */
int sem_post(sem_t *semaphore)
{
	if (semaphore == NULL) {
		errno = EINVAL;
		return -1;
	}

	(void)sys_sem_give(semaphore);
	return 0;
}

/**
 * @brief Try time limited locking a semaphore.
 *
 * See IEEE 1003.1
 */
int sem_timedwait(sem_t *semaphore, struct timespec *abstime)
{
	if ((abstime == NULL) || !timespec_is_valid(abstime)) {
		errno = EINVAL;
		return -1;
	}

	k_timeout_t to = sys_timepoint_timeout(timespec_abs_rt_to_timepoint(abstime));

	if (sys_sem_take(semaphore, to) != 0) {
		errno = ETIMEDOUT;
		return -1;
	}

	return 0;
}

/**
 * @brief Lock a semaphore if not taken.
 *
 * See IEEE 1003.1
 */
int sem_trywait(sem_t *semaphore)
{
	if (sys_sem_take(semaphore, K_NO_WAIT) != 0) {
		errno = EAGAIN;
		return -1;
	}

	return 0;
}

/**
 * @brief Lock a semaphore.
 *
 * See IEEE 1003.1
 */
int sem_wait(sem_t *semaphore)
{
	if (sys_sem_take(semaphore, K_FOREVER) != 0) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

sem_t *sem_open(const char *name, int oflags, ...)
{
	va_list va;
	mode_t mode;
	unsigned int value;
	struct nsem_obj *nsem = NULL;
	size_t namelen;

	va_start(va, oflags);
	BUILD_ASSERT(sizeof(mode_t) <= sizeof(int));
	mode = va_arg(va, int);
	value = va_arg(va, unsigned int);
	va_end(va);

	ARG_UNUSED(mode);

	if (value > CONFIG_POSIX_SEM_VALUE_MAX) {
		errno = EINVAL;
		return (sem_t *)SEM_FAILED;
	}

	if (name == NULL) {
		errno = EINVAL;
		return (sem_t *)SEM_FAILED;
	}

	namelen = strlen(name);
	if ((namelen + 1) > CONFIG_POSIX_SEM_NAMELEN_MAX) {
		errno = ENAMETOOLONG;
		return (sem_t *)SEM_FAILED;
	}

	/* Lock before checking to make sure that the call is atomic */
	nsem_list_lock();

	/* Check if the named semaphore exists */
	nsem = nsem_find(name);

	if (nsem != NULL) { /* Named semaphore exists */
		if (((oflags & O_CREAT) != 0) && ((oflags & O_EXCL) != 0)) {
			errno = EEXIST;
			goto error_unlock;
		}

		__ASSERT_NO_MSG(nsem->ref_count != INT_MAX);
		nsem->ref_count++;
		goto unlock;
	}

	/* Named semaphore doesn't exist, try to create new one */

	if ((oflags & O_CREAT) == 0) {
		errno = ENOENT;
		goto error_unlock;
	}

	if (sys_elastipool_alloc(&nsem_pool, (void **)&nsem) < 0) {
		errno = ENOSPC;
		goto error_unlock;
	}

	strcpy(nsem->name, name);
	nsem->linked = true;

	/* 1 for this open instance, +1 for the linked name */
	nsem->ref_count = 2;

	(void)sys_sem_init(&nsem->sem, value, CONFIG_POSIX_SEM_VALUE_MAX);

	sys_slist_append(&nsem_list, (sys_snode_t *)&(nsem->snode));

	goto unlock;

error_unlock:
	nsem = NULL;

unlock:
	nsem_list_unlock();
	return nsem == NULL ? SEM_FAILED : &nsem->sem;
}

int sem_unlink(const char *name)
{
	int ret = 0;
	struct nsem_obj *nsem;

	if (name == NULL) {
		errno = EINVAL;
		return -1;
	}

	if ((strlen(name) + 1)  > CONFIG_POSIX_SEM_NAMELEN_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}

	nsem_list_lock();

	/* Check if queue already exists */
	nsem = nsem_find(name);
	if (nsem == NULL) {
		ret = -1;
		errno = ENOENT;
		goto unlock;
	}

	nsem->linked = false;
	nsem_unref(nsem);

unlock:
	nsem_list_unlock();
	return ret;
}

int sem_close(sem_t *sem)
{
	struct nsem_obj *nsem;

	if (sem == NULL) {
		errno = EINVAL;
		return -1;
	}

	nsem = CONTAINER_OF(sem, struct nsem_obj, sem);

	nsem_list_lock();
	nsem_unref(nsem);
	nsem_list_unlock();
	return 0;
}

#ifdef CONFIG_ZTEST
/* Used by ztest to get the ref count of a named semaphore */
int nsem_get_ref_count(sem_t *sem)
{
	struct nsem_obj *nsem;
	int ref_count;

	__ASSERT_NO_MSG(sem != NULL);
	nsem = CONTAINER_OF(sem, struct nsem_obj, sem);

	nsem_list_lock();
	ref_count = nsem->ref_count;
	nsem_list_unlock();

	return ref_count;
}

/* Used by ztest to get the length of the named semaphore */
size_t nsem_get_list_len(void)
{
	size_t len;

	nsem_list_lock();
	len = sys_slist_len(&nsem_list);
	nsem_list_unlock();

	return len;
}
#endif

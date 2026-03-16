/*
 * Copyright (c) 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_POSIX_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_POSIX_INTERNAL_H_

#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#include <zephyr/kernel.h>
#include <pthread.h>
#include <zephyr/sys/dlist.h>
#include <zephyr/sys/elastipool.h>
#include <zephyr/sys/slist.h>
#include <zephyr/sys/sem.h>

#define POSIX_OBJ_INITIALIZER (-1)

/*
 * Bit used to mark a pthread object as initialized. Initialization status is
 * verified (against internal status) in lock / unlock / destroy functions.
 */
#define PTHREAD_OBJ_MASK_INIT 0x80000000

struct __packed posix_thread_attr {
	void *stack;
	/* the following two bitfields should combine to be 32-bits in size */
	uint32_t stacksize: CONFIG_POSIX_PTHREAD_ATTR_STACKSIZE_BITS;
	uint16_t guardsize: CONFIG_POSIX_PTHREAD_ATTR_GUARDSIZE_BITS;
	int8_t priority;
	uint8_t schedpolicy: 2;
	bool contentionscope: 1;
	bool inheritsched: 1;
	union {
		bool caller_destroys: 1;
		bool initialized: 1;
	};
	bool cancelpending: 1;
	bool cancelstate: 1;
	bool canceltype: 1;
	bool detachstate: 1;
};

struct posix_thread {
	struct k_thread thread;

	/* List nodes for pthread_cleanup_push() / pthread_cleanup_pop() */
	sys_slist_t cleanup_list;

	/* List node for ready_q, run_q, or done_q */
	sys_dnode_t q_node;

	/* List of keys that thread has called pthread_setspecific() on */
	sys_slist_t key_list;

	/* pthread_attr_t */
	struct posix_thread_attr attr;

	/* Exit status */
	void *retval;

	/* Signal mask */
	sigset_t sigset;

	/* Queue ID (internal-only) */
	uint8_t qid;
};

struct posix_condattr {
	/* leaves room for CLOCK_REALTIME (1, default) and CLOCK_MONOTONIC (4) */
	unsigned char clock: 3;
	char initialized: 1;
#ifdef _POSIX_THREAD_PROCESS_SHARED
	unsigned char pshared: 1;
#endif
};

struct posix_cond {
	struct k_condvar condvar;
	struct posix_condattr attr;
};

typedef struct pthread_key_obj {
	/* List of pthread_key_data objects that contain thread
	 * specific data for the key
	 */
	sys_slist_t key_data_l;

	/* Optional destructor that is passed to pthread_key_create() */
	void (*destructor)(void *value);
} pthread_key_obj;

typedef struct pthread_thread_data {
	sys_snode_t node;

	/* Key and thread specific data passed to pthread_setspecific() */
	pthread_key_obj *key;
	void *spec_data;
} pthread_thread_data;

struct pthread_key_data {
	sys_snode_t node;
	pthread_thread_data thread_data;
};

static inline bool is_pthread_obj_initialized(uint32_t obj)
{
	return (obj & PTHREAD_OBJ_MASK_INIT) != 0;
}

static inline uint32_t mark_pthread_obj_initialized(uint32_t obj)
{
	return obj | PTHREAD_OBJ_MASK_INIT;
}

static inline uint32_t mark_pthread_obj_uninitialized(uint32_t obj)
{
	return obj & ~PTHREAD_OBJ_MASK_INIT;
}

/* get a pointer to a pool object that has already been allocated using handle */
void *posix_get_pool_obj_unlocked(const struct sys_elastipool *pool, uint32_t handle);
/* get a pointer to a pool object if already initialized, otherwise, initialize a new one */
void *posix_init_pool_obj_unlocked(const struct sys_elastipool *pool, uint32_t handle,
				   void (*cb)(void *obj));

static inline void *posix_get_pool_obj(const struct sys_elastipool *pool, struct sys_sem *lock,
				       uint32_t handle)
{
	void *ret = NULL;

	SYS_SEM_LOCK(lock) {
		ret = posix_get_pool_obj_unlocked(pool, handle);
	}

	return (void *)ret;
}

static inline void *posix_init_pool_obj(const struct sys_elastipool *pool, struct sys_sem *lock,
					uint32_t handle, void (*cb)(void *obj))
{
	void *ret = NULL;

	SYS_SEM_LOCK(lock) {
		ret = posix_init_pool_obj_unlocked(pool, handle, cb);
	}

	return (void *)ret;
}

struct posix_thread *to_posix_thread(pthread_t pth);

/* get and possibly initialize a posix_mutex */
struct k_mutex *to_posix_mutex(pthread_mutex_t *mu);

int posix_to_zephyr_priority(int priority, int policy);
int zephyr_to_posix_priority(int priority, int *policy);

BUILD_ASSERT((sizeof(void *) == sizeof(pthread_barrier_t)) ||
		     (sizeof(void *) == 2 * sizeof(pthread_barrier_t)),
	     "unsupported pthread_barrier_t size");
BUILD_ASSERT((sizeof(void *) == sizeof(pthread_cond_t)) ||
		     (sizeof(void *) == 2 * sizeof(pthread_cond_t)),
	     "unsupported pthread_cond_t size");
BUILD_ASSERT((sizeof(void *) == sizeof(pthread_key_t)) ||
		     (sizeof(void *) == 2 * sizeof(pthread_key_t)),
	     "unsupported pthread_key_t size");
BUILD_ASSERT((sizeof(void *) == sizeof(pthread_mutex_t)) ||
		     (sizeof(void *) == 2 * sizeof(pthread_mutex_t)),
	     "unsupported pthread_mutex_t size");

/* FIXME: Need to adjust the toolchain so that pthread_t, pthread_mutex_t, pthread_cond_t,
 * pthread_key_t, etc are the same size of uintptr_t (i.e. void *) */
static inline void *posix_to_kernel_object(void *input, size_t size, void *ref)
{
	void *output;

	if (sizeof(void *) == size) {
		output = (void *)*((uintptr_t *)input);
	} else if ((sizeof(void *) == 2 * size) && (sizeof(uint32_t) == size)) {
		output = (void *)(uintptr_t)(((uintptr_t)ref & GENMASK64(63, 32)) |
					     *(uint32_t *)input);
	}

	return output;
}

static inline uintptr_t posix_from_kernel_object(void *input, size_t size)
{
	uintptr_t output;

	if (sizeof(void *) == size) {
		output = (uintptr_t)input;
	} else if (sizeof(void *) == 2 * size) {
		output = (uintptr_t)input & GENMASK64(31, 0);
	}

	return output;
}

static inline struct k_mutex *to_k_mutex(const pthread_mutex_t *mu)
{
	extern struct k_mutex sys_mutex_pool[];

	return (struct k_mutex *)posix_to_kernel_object((void *)mu, sizeof(pthread_mutex_t),
							sys_mutex_pool);
}

static inline pthread_mutex_t to_pthread_mutex(const struct k_mutex *kmu)
{
	return (pthread_mutex_t)posix_from_kernel_object((void *)kmu, sizeof(pthread_mutex_t));
}

static inline struct k_condvar *to_k_condvar(const pthread_cond_t *cv)
{
	extern struct k_condvar sys_condvar_pool[];

	return (struct k_condvar *)posix_to_kernel_object((void *)cv, sizeof(pthread_cond_t),
							  sys_condvar_pool);
}

static inline pthread_cond_t to_pthread_cond(const struct k_condvar *kcv)
{
	return (pthread_cond_t)posix_from_kernel_object((void *)kcv, sizeof(pthread_cond_t));
}

static inline struct k_thread *to_k_thread(const pthread_t *th)
{
	extern struct k_thread sys_thread_pool[];

	return (struct k_thread *)posix_to_kernel_object((void *)th, sizeof(pthread_t),
							 sys_thread_pool);
}

static inline pthread_t to_pthread_thread(const struct k_thread *kth)
{
	return (pthread_t)posix_from_kernel_object((void *)kth, sizeof(pthread_t));
}

#endif

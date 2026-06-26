/*
 * Copyright (c) 2025 The Zephyr Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS__PTHREADTYPES_H_
#define ZEPHYR_INCLUDE_POSIX_SYS__PTHREADTYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/sys/atomic_types.h>
#include <zephyr/sys/condvar.h>
#include <zephyr/sys/mutex.h>
#include <zephyr/sys/thread_once.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_PTHREAD_ATTR_T_DECLARED) || defined(__pthread_attr_t_defined)) ||                   \
	defined(__DOXYGEN__)
typedef struct {
	void *stack;
	size_t stacksize;
	size_t guardsize;
	uint32_t details;
} pthread_attr_t;
#define _PTHREAD_ATTR_T_DECLARED
#define __pthread_attr_t_defined
#endif

#if !(defined(_PTHREAD_BARRIER_T_DECLARED) && defined(__pthread_barrier_t_defined)) ||             \
	defined(__DOXYGEN__)
typedef uint32_t pthread_barrier_t;
#define _PTHREAD_BARRIER_T_DECLARED
#define __pthread_barrier_t_defined
#endif

#if !(defined(_PTHREAD_BARRIERATTR_T_DECLARED) && defined(__pthread_barrierattr_t_defined)) ||     \
	defined(__DOXYGEN__)
typedef struct {
	int pshared;
} pthread_barrierattr_t;
#define _PTHREAD_BARRIERATTR_T_DECLARED
#define __pthread_barrierattr_t_defined
#endif

#if !(defined(_PTHREAD_COND_T_DECLARED) && defined(__pthread_cond_t_defined)) ||                   \
	defined(__DOXYGEN__)
typedef struct sys_condvar pthread_cond_t;
#define _PTHREAD_COND_T_DECLARED
#define __pthread_cond_t_defined
#endif

#if !(defined(_PTHREAD_CONDATTR_T_DECLARED) && defined(__pthread_condattr_t_defined)) ||           \
	defined(__DOXYGEN__)
typedef struct {
	clockid_t clock;
} pthread_condattr_t;
#define _PTHREAD_CONDATTR_T_DECLARED
#define __pthread_condattr_t_defined
#endif

#if !(defined(_PTHREAD_KEY_T_DECLARED) && defined(__pthread_key_t_defined)) || defined(__DOXYGEN__)
typedef uintptr_t pthread_key_t;
#define _PTHREAD_KEY_T_DECLARED
#define __pthread_key_t_defined
#endif

#if !(defined(_PTHREAD_MUTEX_T_DECLARED) && defined(__pthread_mutex_t_defined)) ||                 \
	defined(__DOXYGEN__)
typedef struct sys_mutex pthread_mutex_t;
#define _PTHREAD_MUTEX_T_DECLARED
#define __pthread_mutex_t_defined
#endif

#if !(defined(_PTHREAD_MUTEXATTR_T_DECLARED) && defined(__pthread_mutexattr_t_defined)) ||         \
	defined(__DOXYGEN__)
typedef struct {
	unsigned char type: 2;
	bool initialized: 1;
} pthread_mutexattr_t;
#define _PTHREAD_MUTEXATTR_T_DECLARED
#define __pthread_mutexattr_t_defined
#endif

#if !(defined(_PTHREAD_ONCE_T_DECLARED) && defined(__pthread_once_t_defined)) ||                   \
	defined(__DOXYGEN__)
typedef sys_thread_once_t pthread_once_t;
#define _PTHREAD_ONCE_T_DECLARED
#define __pthread_once_t_defined
#endif

#if !(defined(_PTHREAD_RWLOCK_T_DECLARED) && defined(__pthread_rwlock_t_defined)) ||               \
	defined(__DOXYGEN__)
typedef uint32_t pthread_rwlock_t;
#define _PTHREAD_RWLOCK_T_DECLARED
#define __pthread_rwlock_t_defined
#endif

#if !(defined(_PTHREAD_RWLOCKATTR_T_DECLARED) && defined(__pthread_rwlockattr_t_defined)) ||       \
	defined(__DOXYGEN__)
typedef uint32_t pthread_rwlockattr_t;
#define _PTHREAD_RWLOCKATTR_T_DECLARED
#define __pthread_rwlockattr_t_defined
#endif

#if !(defined(_PTHREAD_SPINLOCK_T_DECLARED) && defined(__pthread_spinlock_t_defined)) ||           \
	defined(__DOXYGEN__)
/* TODO: convert this to a long so that it can refer to a sys_sem_t (pointer) */
typedef uint32_t pthread_spinlock_t;
#define _PTHREAD_SPINLOCK_T_DECLARED
#define __pthread_spinlock_t_defined
#endif

#if !(defined(_PTHREAD_T_DECLARED) && defined(__pthread_t_defined)) || defined(__DOXYGEN__)
/* TODO: convert this to a long so that it can refer to a k_thread (pointer) */
typedef uint32_t pthread_t;
#define _PTHREAD_T_DECLARED
#define __pthread_t_defined
#endif

/* clang-format off */
#ifndef _PTHREAD_MUTEX_INITIALIZER
/* a struct sys_mutex whose options are K_MUTEX_NORMAL */
#define _PTHREAD_MUTEX_INITIALIZER {0, 0, 0, 0, 1}
#endif
#ifndef _PTHREAD_COND_INITIALIZER
#define _PTHREAD_COND_INITIALIZER {0}
#endif
/* clang-format on */

#ifndef _PTHREAD_RWLOCK_INITIALIZER
#define _PTHREAD_RWLOCK_INITIALIZER (-1)
#endif

#ifndef _PTHREAD_ONCE_INITIALIZER
/* clang-format off */
#define _PTHREAD_ONCE_INITIALIZER {0}
/* clang-format on */
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS__PTHREADTYPES_H_ */

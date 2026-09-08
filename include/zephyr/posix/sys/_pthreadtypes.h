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
/* futex-backed layout: a struct k_futex (sequence word, kernel-maintained waiter count) */
struct posix_futex_cond {
	atomic_t seq;
	atomic_t waiters;
	uint32_t flags;
};
#ifdef CONFIG_POSIX_THREAD_FUTEX
typedef struct posix_futex_cond pthread_cond_t;
#else
/* TODO: convert this to a long so that it can refer to a k_condvar (pointer) */
typedef uint32_t pthread_cond_t;
#endif
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
/* futex-backed layout: a struct k_futex (lock word, kernel-maintained waiter count) */
struct posix_futex_mutex {
	atomic_t val;
	atomic_t waiters;
	uint32_t owner;
	uint16_t count;
	uint8_t type;
	uint8_t protocol;
};
#ifdef CONFIG_POSIX_THREAD_FUTEX
typedef struct posix_futex_mutex pthread_mutex_t;
#else
/* TODO: convert this to a long so that it can refer to a k_mutex (pointer) */
typedef uint32_t pthread_mutex_t;
#endif
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
typedef struct {
	unsigned long flag;
} pthread_once_t;
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

/* kernel-object handle types: an out-of-range handle marks a statically initialized object */
#ifndef _PTHREAD_HANDLE_INITIALIZER
#define _PTHREAD_HANDLE_INITIALIZER (-1)
#endif

/* clang-format off */
#ifdef CONFIG_POSIX_THREAD_FUTEX
#ifndef _PTHREAD_MUTEX_INITIALIZER
#define _PTHREAD_MUTEX_INITIALIZER {0}
#endif
#ifndef _PTHREAD_COND_INITIALIZER
#define _PTHREAD_COND_INITIALIZER {0}
#endif
#else
#ifndef _PTHREAD_MUTEX_INITIALIZER
#define _PTHREAD_MUTEX_INITIALIZER _PTHREAD_HANDLE_INITIALIZER
#endif
#ifndef _PTHREAD_COND_INITIALIZER
#define _PTHREAD_COND_INITIALIZER _PTHREAD_HANDLE_INITIALIZER
#endif
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

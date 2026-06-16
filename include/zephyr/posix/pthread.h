/*
 * SPDX-FileCopyrightText: Copyright (c) The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX threads API (<pthread.h>)
 *
 * Covers thread lifecycle, attributes, cancellation, scheduling, mutexes,
 * condition variables, barriers, reader-writer locks, spin locks, and
 * thread-specific data.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pthread.h.html">
 *      POSIX.1-2017 &lt;pthread.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_PTHREAD_H_
#define ZEPHYR_INCLUDE_POSIX_PTHREAD_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <time.h>
#include <sched.h>

#include <zephyr/posix/sys/_pthreadtypes.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @brief Returned by pthread_barrier_wait() to one (arbitrary) thread per barrier cycle.  @ingroup posix_option_group_barriers*/
#define PTHREAD_BARRIER_SERIAL_THREAD 1
/** @brief Thread cancellation is performed immediately.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_CANCEL_ASYNCHRONOUS   1
/** @brief Cancellation is enabled (default).  @ingroup posix_option_group_threads_base*/
#define PTHREAD_CANCEL_ENABLE         0
/** @brief Cancellation is deferred until a cancellation point (default).  @ingroup posix_option_group_threads_base*/
#define PTHREAD_CANCEL_DEFERRED       0
/** @brief Cancellation delivery is disabled.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_CANCEL_DISABLE        1
/** @brief Value returned by pthread_join() for a cancelled thread.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_CANCELED              ((void *)-1)
/** @brief Thread is created in the detached state.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_CREATE_DETACHED       1
/** @brief Thread is created in the joinable state (default).  @ingroup posix_option_group_threads_base*/
#define PTHREAD_CREATE_JOINABLE       0
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/** @brief Thread uses an explicitly provided scheduling policy.  @ingroup posix_option_thread_priority_scheduling*/
#define PTHREAD_EXPLICIT_SCHED 1
/** @brief Thread inherits the scheduling policy of its creator (default).  @ingroup posix_option_thread_priority_scheduling*/
#define PTHREAD_INHERIT_SCHED  0
#endif
/** @brief Default mutex type; behaviour on deadlock/unlock-by-non-owner is undefined.  @ingroup posix_option_group_posix_threads_ext*/
#define PTHREAD_MUTEX_DEFAULT       3
/** @brief Mutex that returns an error on deadlock or unlock by a non-owner.  @ingroup posix_option_group_posix_threads_ext*/
#define PTHREAD_MUTEX_ERRORCHECK    2
/** @brief Non-recursive mutex with no error checks (fastest).  @ingroup posix_option_group_posix_threads_ext*/
#define PTHREAD_MUTEX_NORMAL        0
/** @brief Recursive mutex; the owning thread may lock it multiple times.  @ingroup posix_option_group_posix_threads_ext*/
#define PTHREAD_MUTEX_RECURSIVE     1
/** @brief Robust mutex; the state is recoverable if the owner dies. @ingroup posix_option_group_robust_mutexes */
#define PTHREAD_MUTEX_ROBUST        4
/** @brief Mutex is not recovered on owner death (default, for non-robust mutexes). @ingroup posix_option_group_robust_mutexes */
#define PTHREAD_MUTEX_STALLED       1
/** @brief Static initialiser for pthread_once_t objects.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_ONCE_INIT           _PTHREAD_ONCE_INITIALIZER
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(__DOXYGEN__)
/** @brief Mutex protocol: owning thread inherits priority of highest-priority waiter.  @ingroup posix_option_thread_prio_inherit*/
#define PTHREAD_PRIO_INHERIT 1
#endif
#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT) ||                  \
	defined(__DOXYGEN__)
/** @brief Mutex protocol: no priority inheritance or ceiling.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_PRIO_NONE 0
#endif
#if defined(_POSIX_THREAD_PRIO_PROTECT) || defined(__DOXYGEN__)
/** @brief Mutex protocol: owner runs at ceiling priority while holding the mutex.  @ingroup posix_option_thread_prio_protect*/
#define PTHREAD_PRIO_PROTECT 2
#endif
/** @brief Mutex or barrier attribute: object is shared between processes. @ingroup posix_option_thread_process_shared */
#define PTHREAD_PROCESS_SHARED  1
/** @brief Mutex or barrier attribute: object is private to the process (default). @ingroup posix_option_thread_process_shared */
#define PTHREAD_PROCESS_PRIVATE 0
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/** @brief Thread competes for resources only with threads in the same process.  @ingroup posix_option_thread_priority_scheduling*/
#define PTHREAD_SCOPE_PROCESS 1
/** @brief Thread competes for resources with all threads in the system (default).  @ingroup posix_option_thread_priority_scheduling*/
#define PTHREAD_SCOPE_SYSTEM  0
#endif

/** @brief Static initialiser for pthread_cond_t objects.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_COND_INITIALIZER   _PTHREAD_COND_INITIALIZER
/** @brief Static initialiser for pthread_mutex_t objects.  @ingroup posix_option_group_threads_base*/
#define PTHREAD_MUTEX_INITIALIZER  _PTHREAD_MUTEX_INITIALIZER
/** @brief Static initialiser for pthread_rwlock_t objects.  @ingroup posix_option_group_rw_locks*/
#define PTHREAD_RWLOCK_INITIALIZER _PTHREAD_RWLOCK_INITIALIZER

/* pthread_attr_t, pthread_barrier_t, pthread_barrierattr_t, pthread_cond_t, pthread_condattr_t,
 * pthread_key_t, pthread_mutex_t, pthread_mutexattr_t, pthread_once_t, pthread_rwlock_t,
 * pthread_rwlockattr_t, pthread_spinlock_t, and pthread_t
 * types defined in <zephyr/posix/sys/_pthreadtypes.h>
 */

/**
 * @brief Register fork handlers to be called around fork().
 * @ingroup posix_option_group_multi_process
 *
 * @param prepare Handler called in the parent before fork().
 * @param parent  Handler called in the parent after fork().
 * @param child   Handler called in the child after fork().
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_atfork.html
 */
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));

/**
 * @brief Destroy a thread attributes object.
 * @ingroup posix_option_group_threads_base
 * @param attr Thread attributes object to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_destroy.html
 */
int pthread_attr_destroy(pthread_attr_t *attr);

/**
 * @brief Get the detach state attribute of a thread attributes object.
 * @ingroup posix_option_group_threads_base
 * @param attr        Thread attributes object.
 * @param detachstate Output: @c PTHREAD_CREATE_DETACHED or @c PTHREAD_CREATE_JOINABLE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getdetachstate.html
 */
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate);

/**
 * @brief Get the guard size attribute of a thread attributes object.
 * @ingroup posix_option_group_posix_threads_ext
 * @param attr      Thread attributes object.
 * @param guardsize Output: guard size in bytes.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getguardsize.html
 */
int pthread_attr_getguardsize(const pthread_attr_t *ZRESTRICT attr, size_t *ZRESTRICT guardsize);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/**
 * @brief Get the inherit-scheduler attribute of a thread attributes object.
 * @ingroup posix_option_thread_priority_scheduling
 * @param attr         Thread attributes object.
 * @param inheritsched Output: @c PTHREAD_INHERIT_SCHED or @c PTHREAD_EXPLICIT_SCHED.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getinheritsched.html
 */
int pthread_attr_getinheritsched(const pthread_attr_t *ZRESTRICT attr,
				 int *ZRESTRICT inheritsched);
#endif

/**
 * @brief Get the scheduling parameter attribute of a thread attributes object.
 * @ingroup posix_option_group_threads_base
 * @param attr  Thread attributes object.
 * @param param Output: scheduling parameters.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getschedparam.html
 */
int pthread_attr_getschedparam(const pthread_attr_t *ZRESTRICT attr,
			       struct sched_param *ZRESTRICT param);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/**
 * @brief Get the scheduling policy attribute of a thread attributes object.
 * @ingroup posix_option_thread_priority_scheduling
 * @param attr   Thread attributes object.
 * @param policy Output: @c SCHED_FIFO, @c SCHED_RR, or @c SCHED_OTHER.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getschedpolicy.html
 */
int pthread_attr_getschedpolicy(const pthread_attr_t *ZRESTRICT attr, int *ZRESTRICT policy);

/**
 * @brief Get the contention scope attribute of a thread attributes object.
 * @ingroup posix_option_thread_priority_scheduling
 * @param attr            Thread attributes object.
 * @param contentionscope Output: @c PTHREAD_SCOPE_SYSTEM or @c PTHREAD_SCOPE_PROCESS.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getscope.html
 */
int pthread_attr_getscope(const pthread_attr_t *ZRESTRICT attr, int *ZRESTRICT contentionscope);
#endif

#if (defined(_POSIX_THREAD_ATTR_STACKADDR) && defined(_POSIX_THREAD_ATTR_STACKSIZE)) ||          \
	defined(__DOXYGEN__)
/**
 * @brief Get the stack address and size attributes of a thread attributes object.
 * @ingroup posix_option_thread_attr_stackaddr
 * @param attr      Thread attributes object.
 * @param stackaddr Output: stack base address.
 * @param stacksize Output: stack size in bytes.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getstack.html
 */
int pthread_attr_getstack(const pthread_attr_t *ZRESTRICT attr, void **ZRESTRICT stackaddr,
			  size_t *ZRESTRICT stacksize);
#endif

#if defined(_POSIX_THREAD_ATTR_STACKSIZE) || defined(__DOXYGEN__)
/**
 * @brief Get the stack size attribute of a thread attributes object.
 * @ingroup posix_option_thread_attr_stacksize
 * @param attr      Thread attributes object.
 * @param stacksize Output: stack size in bytes.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_getstacksize.html
 */
int pthread_attr_getstacksize(const pthread_attr_t *ZRESTRICT attr, size_t *ZRESTRICT stacksize);
#endif

/**
 * @brief Initialise a thread attributes object with default values.
 * @ingroup posix_option_group_threads_base
 * @param attr Thread attributes object to initialise.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_init.html
 */
int pthread_attr_init(pthread_attr_t *attr);

/**
 * @brief Set the detach state attribute of a thread attributes object.
 * @ingroup posix_option_group_threads_base
 * @param attr        Thread attributes object.
 * @param detachstate @c PTHREAD_CREATE_DETACHED or @c PTHREAD_CREATE_JOINABLE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setdetachstate.html
 */
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);

/**
 * @brief Set the guard size attribute of a thread attributes object.
 * @ingroup posix_option_group_posix_threads_ext
 * @param attr      Thread attributes object.
 * @param guardsize Guard size in bytes.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setguardsize.html
 */
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/**
 * @brief Set the inherit-scheduler attribute of a thread attributes object.
 * @ingroup posix_option_thread_priority_scheduling
 * @param attr         Thread attributes object.
 * @param inheritsched @c PTHREAD_INHERIT_SCHED or @c PTHREAD_EXPLICIT_SCHED.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setinheritsched.html
 */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);
#endif

/**
 * @brief Set the scheduling parameter attribute of a thread attributes object.
 * @ingroup posix_option_group_threads_base
 * @param attr  Thread attributes object.
 * @param param Scheduling parameters to apply.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setschedparam.html
 */
int pthread_attr_setschedparam(pthread_attr_t *ZRESTRICT attr,
			       const struct sched_param *ZRESTRICT param);

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/**
 * @brief Set the scheduling policy attribute of a thread attributes object.
 * @ingroup posix_option_thread_priority_scheduling
 * @param attr   Thread attributes object.
 * @param policy @c SCHED_FIFO, @c SCHED_RR, or @c SCHED_OTHER.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setschedpolicy.html
 */
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);

/**
 * @brief Set the contention scope attribute of a thread attributes object.
 * @ingroup posix_option_thread_priority_scheduling
 * @param attr            Thread attributes object.
 * @param contentionscope @c PTHREAD_SCOPE_SYSTEM or @c PTHREAD_SCOPE_PROCESS.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setscope.html
 */
int pthread_attr_setscope(pthread_attr_t *attr, int contentionscope);
#endif

#if (defined(_POSIX_THREAD_ATTR_STACKADDR) && defined(_POSIX_THREAD_ATTR_STACKSIZE)) ||          \
	defined(__DOXYGEN__)
/**
 * @brief Set the stack address and size attributes of a thread attributes object.
 * @ingroup posix_option_thread_attr_stackaddr
 * @param attr      Thread attributes object.
 * @param stackaddr Stack base address.
 * @param stacksize Stack size in bytes (>= PTHREAD_STACK_MIN).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setstack.html
 */
int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize);
#endif

#if defined(_POSIX_THREAD_ATTR_STACKSIZE) || defined(__DOXYGEN__)
/**
 * @brief Set the stack size attribute of a thread attributes object.
 * @ingroup posix_option_thread_attr_stacksize
 * @param attr      Thread attributes object.
 * @param stacksize Stack size in bytes (>= PTHREAD_STACK_MIN).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_attr_setstacksize.html
 */
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
#endif



/**
 * @brief Destroy a barrier object.
 * @ingroup posix_option_group_barriers
 * @param barrier Barrier to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrier_destroy.html
 */
int pthread_barrier_destroy(pthread_barrier_t *barrier);

/**
 * @brief Initialise a barrier object.
 * @ingroup posix_option_group_barriers
 * @param barrier Barrier to initialise.
 * @param attr    Barrier attributes, or NULL for defaults.
 * @param count   Number of threads that must call pthread_barrier_wait() before any proceeds.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrier_init.html
 */
int pthread_barrier_init(pthread_barrier_t *ZRESTRICT barrier,
			 const pthread_barrierattr_t *ZRESTRICT attr, unsigned int count);

/**
 * @brief Synchronise participating threads at a barrier.
 * @ingroup posix_option_group_barriers
 *
 * Blocks until @p count threads have called this function on the same barrier.
 * Exactly one thread receives @c PTHREAD_BARRIER_SERIAL_THREAD as the return value.
 *
 * @param barrier Barrier to wait on.
 * @return @c PTHREAD_BARRIER_SERIAL_THREAD for one thread, 0 for all others,
 *         or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrier_wait.html
 */
int pthread_barrier_wait(pthread_barrier_t *barrier);

/**
 * @brief Destroy a barrier attributes object.
 * @ingroup posix_option_group_barriers
 * @param attr Barrier attributes object to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrierattr_destroy.html
 */
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr);

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Get the process-shared attribute of a barrier attributes object.
 * @ingroup posix_option_group_barriers
 * @param attr    Barrier attributes object.
 * @param pshared Output: @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrierattr_getpshared.html
 */
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *ZRESTRICT attr,
				   int *ZRESTRICT pshared);
#endif

/**
 * @brief Initialise a barrier attributes object with default values.
 * @ingroup posix_option_group_barriers
 * @param attr Barrier attributes object to initialise.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrierattr_init.html
 */
int pthread_barrierattr_init(pthread_barrierattr_t *attr);

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Set the process-shared attribute of a barrier attributes object.
 * @ingroup posix_option_group_barriers
 * @param attr    Barrier attributes object.
 * @param pshared @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_barrierattr_setpshared.html
 */
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared);
#endif



/**
 * @brief Send a cancellation request to a thread.
 * @ingroup posix_option_group_threads_base
 * @param thread Thread to cancel.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cancel.html
 */
int pthread_cancel(pthread_t thread);



/**
 * @brief Broadcast a condition variable, waking all waiting threads.
 * @ingroup posix_option_group_threads_base
 * @param cond Condition variable to broadcast.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_broadcast.html
 */
int pthread_cond_broadcast(pthread_cond_t *cond);

#if (_POSIX_C_SOURCE >= 202405L) || defined(__DOXYGEN__)
/**
 * @brief Wait on a condition variable with a specific clock.
 * @ingroup posix_option_group_clock_selection
 * @param cond     Condition variable.
 * @param mutex    Associated mutex (must be locked by the caller).
 * @param clock_id Clock used to interpret @p abstime.
 * @param abstime  Absolute timeout.
 * @return 0 on success, @c ETIMEDOUT on timeout, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_clockwait.html
 */
int pthread_cond_clockwait(pthread_cond_t *ZRESTRICT cond, pthread_mutex_t *ZRESTRICT mutex,
	clockid_t clock_id, const struct timespec *ZRESTRICT abstime);
#endif

/**
 * @brief Destroy a condition variable.
 * @ingroup posix_option_group_threads_base
 * @param cond Condition variable to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_destroy.html
 */
int pthread_cond_destroy(pthread_cond_t *cond);

/**
 * @brief Initialise a condition variable.
 * @ingroup posix_option_group_threads_base
 * @param cond Condition variable to initialise.
 * @param attr Condition variable attributes, or NULL for defaults.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_init.html
 */
int pthread_cond_init(pthread_cond_t *ZRESTRICT cond, const pthread_condattr_t *ZRESTRICT attr);

/**
 * @brief Signal a condition variable, waking at least one waiting thread.
 * @ingroup posix_option_group_threads_base
 * @param cond Condition variable to signal.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_signal.html
 */
int pthread_cond_signal(pthread_cond_t *cond);

#if defined(_POSIX_TIMEOUTS) || defined(__DOXYGEN__)
/**
 * @brief Wait on a condition variable with an absolute timeout (CLOCK_REALTIME).
 * @ingroup posix_option_group_threads_base
 * @param cond    Condition variable.
 * @param mutex   Associated mutex (must be locked by the caller).
 * @param abstime Absolute timeout (CLOCK_REALTIME).
 * @return 0 on success, @c ETIMEDOUT on timeout, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_timedwait.html
 */
int pthread_cond_timedwait(pthread_cond_t *ZRESTRICT cond, pthread_mutex_t *ZRESTRICT mutex,
			   const struct timespec *ZRESTRICT abstime);
#endif

/**
 * @brief Wait on a condition variable.
 * @ingroup posix_option_group_threads_base
 * @param cond  Condition variable.
 * @param mutex Associated mutex (must be locked by the caller).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_wait.html
 */
int pthread_cond_wait(pthread_cond_t *ZRESTRICT cond, pthread_mutex_t *ZRESTRICT mutex);

/**
 * @brief Destroy a condition variable attributes object.
 * @ingroup posix_option_group_threads_base
 * @param attr Condition variable attributes object to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_condattr_destroy.html
 */
int pthread_condattr_destroy(pthread_condattr_t *attr);

/**
 * @brief Get the clock attribute of a condition variable attributes object.
 * @ingroup posix_option_group_clock_selection
 * @param attr     Condition variable attributes object.
 * @param clock_id Output: clock ID.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_condattr_getclock.html
 */
int pthread_condattr_getclock(const pthread_condattr_t *ZRESTRICT attr,
			      clockid_t *ZRESTRICT clock_id);

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Get the process-shared attribute of a condition variable attributes object.
 * @ingroup posix_option_thread_process_shared
 * @param attr    Condition variable attributes object.
 * @param pshared Output: @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_condattr_getpshared.html
 */
int pthread_condattr_getpshared(const pthread_condattr_t *ZRESTRICT attr, int *ZRESTRICT pshared);
#endif

/**
 * @brief Initialise a condition variable attributes object with default values.
 * @ingroup posix_option_group_threads_base
 * @param attr Condition variable attributes object to initialise.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_condattr_init.html
 */
int pthread_condattr_init(pthread_condattr_t *attr);

/**
 * @brief Set the clock attribute of a condition variable attributes object.
 * @ingroup posix_option_group_clock_selection
 * @param attr     Condition variable attributes object.
 * @param clock_id Clock ID (e.g. @c CLOCK_MONOTONIC or @c CLOCK_REALTIME).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_condattr_setclock.html
 */
int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id);

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Set the process-shared attribute of a condition variable attributes object.
 * @ingroup posix_option_thread_process_shared
 * @param attr    Condition variable attributes object.
 * @param pshared @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_condattr_setpshared.html
 */
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared);
#endif



/**
 * @brief Create a new thread.
 * @ingroup posix_option_group_threads_base
 * @param thread        Output: thread ID of the new thread.
 * @param attr          Thread attributes, or NULL for defaults.
 * @param start_routine Thread entry point function.
 * @param arg           Argument passed to @p start_routine.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html
 */
int pthread_create(pthread_t *ZRESTRICT thread, const pthread_attr_t *ZRESTRICT attr,
		   void *(*start_routine)(void *), void *ZRESTRICT arg);

/**
 * @brief Detach a thread, releasing its resources automatically on termination.
 * @ingroup posix_option_group_threads_base
 * @param thread Thread to detach.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_detach.html
 */
int pthread_detach(pthread_t thread);

/**
 * @brief Compare two thread IDs.
 * @ingroup posix_option_group_threads_base
 * @param t1 First thread ID.
 * @param t2 Second thread ID.
 * @return Non-zero if @p t1 and @p t2 refer to the same thread, 0 otherwise.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_equal.html
 */
int pthread_equal(pthread_t t1, pthread_t t2);

/**
 * @brief Terminate the calling thread.
 * @ingroup posix_option_group_threads_base
 * @param value_ptr Value made available to pthread_join().
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_exit.html
 */
void pthread_exit(void *value_ptr);

#if (_XOPEN_SOURCE < 800) || defined(__DOXYGEN__)
/**
 * @brief Get the concurrency level (advisory, has no effect on scheduling).
 * @ingroup posix_option_group_xsi_threads_ext
 * @return Current concurrency level.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getconcurrency.html
 */
int pthread_getconcurrency(void);
#endif

#if defined(_POSIX_THREAD_CPUTIME) || defined(__DOXYGEN__)
/**
 * @brief Get the CPU-time clock of a thread.
 * @ingroup posix_option_thread_cputime
 * @param thread_id Thread whose CPU-time clock to retrieve.
 * @param clock_id  Output: clock ID for the thread.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getcpuclockid.html
 */
int pthread_getcpuclockid(pthread_t thread_id, clockid_t *clock_id);
#endif

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/**
 * @brief Get the scheduling policy and parameters of a thread.
 * @ingroup posix_option_thread_priority_scheduling
 * @param thread Thread to query.
 * @param policy Output: scheduling policy.
 * @param param  Output: scheduling parameters.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getschedparam.html
 */
int pthread_getschedparam(pthread_t thread, int *ZRESTRICT policy,
			  struct sched_param *ZRESTRICT param);
#endif

/**
 * @brief Get the thread-specific value associated with a key.
 * @ingroup posix_option_group_threads_base
 * @param key Thread-specific data key.
 * @return Value associated with @p key for the calling thread, or NULL if none.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getspecific.html
 */
void *pthread_getspecific(pthread_key_t key);

/**
 * @brief Wait for a thread to terminate and retrieve its exit status.
 * @ingroup posix_option_group_threads_base
 * @param thread    Thread to wait for.
 * @param value_ptr Output: exit value or @c PTHREAD_CANCELED.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_join.html
 */
int pthread_join(pthread_t thread, void **value_ptr);

/**
 * @brief Create a thread-specific data key.
 * @ingroup posix_option_group_threads_base
 * @param key        Output: new key.
 * @param destructor Destructor called with the thread's value when the thread exits, or NULL.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_key_create.html
 */
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));

/**
 * @brief Delete a thread-specific data key.
 * @ingroup posix_option_group_threads_base
 * @param key Key to delete.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_key_delete.html
 */
int pthread_key_delete(pthread_key_t key);



/**
 * @brief Mark a robust mutex as consistent after recovering from owner death.
 * @ingroup posix_option_group_robust_mutexes
 * @param mutex Robust mutex to mark consistent.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_consistent.html
 */
int pthread_mutex_consistent(pthread_mutex_t *mutex);

/**
 * @brief Destroy a mutex.
 * @ingroup posix_option_group_threads_base
 * @param mutex Mutex to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_destroy.html
 */
int pthread_mutex_destroy(pthread_mutex_t *mutex);

#if defined(_POSIX_THREAD_PRIO_PROTECT) || defined(__DOXYGEN__)
/**
 * @brief Get the priority ceiling of a mutex.
 * @ingroup posix_option_thread_prio_protect
 * @param mutex       Mutex to query.
 * @param prioceiling Output: priority ceiling value.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_getprioceiling.html
 */
int pthread_mutex_getprioceiling(const pthread_mutex_t *ZRESTRICT mutex,
				 int *ZRESTRICT prioceiling);
#endif

/**
 * @brief Initialise a mutex.
 * @ingroup posix_option_group_threads_base
 * @param mutex Mutex to initialise.
 * @param attr  Mutex attributes, or NULL for defaults.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_init.html
 */
int pthread_mutex_init(pthread_mutex_t *ZRESTRICT mutex, const pthread_mutexattr_t *ZRESTRICT attr);

/**
 * @brief Lock a mutex, blocking until it is available.
 * @ingroup posix_option_group_threads_base
 * @param mutex Mutex to lock.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html
 */
int pthread_mutex_lock(pthread_mutex_t *mutex);

#if defined(_POSIX_THREAD_PRIO_PROTECT) || defined(__DOXYGEN__)
/**
 * @brief Set the priority ceiling of a mutex.
 * @ingroup posix_option_thread_prio_protect
 * @param mutex       Mutex to update.
 * @param prioceiling New priority ceiling value.
 * @param old_ceiling Output: previous priority ceiling, or NULL.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_setprioceiling.html
 */
int pthread_mutex_setprioceiling(pthread_mutex_t *ZRESTRICT mutex, int prioceiling,
				 int *ZRESTRICT old_ceiling);
#endif

#if defined(_POSIX_TIMEOUTS) || defined(__DOXYGEN__)
/**
 * @brief Lock a mutex with an absolute timeout.
 * @ingroup posix_option_group_threads_base
 * @param mutex   Mutex to lock.
 * @param abstime Absolute timeout (CLOCK_REALTIME).
 * @return 0 on success, @c ETIMEDOUT on timeout, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_timedlock.html
 */
int pthread_mutex_timedlock(pthread_mutex_t *ZRESTRICT mutex,
			    const struct timespec *ZRESTRICT abstime);
#endif

/**
 * @brief Try to lock a mutex without blocking.
 * @ingroup posix_option_group_threads_base
 * @param mutex Mutex to try to lock.
 * @return 0 if the lock was acquired, @c EBUSY if already locked, or a positive error number.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_trylock.html
 */
int pthread_mutex_trylock(pthread_mutex_t *mutex);

/**
 * @brief Unlock a mutex.
 * @ingroup posix_option_group_threads_base
 * @param mutex Mutex to unlock.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_unlock.html
 */
int pthread_mutex_unlock(pthread_mutex_t *mutex);

/**
 * @brief Destroy a mutex attributes object.
 * @ingroup posix_option_group_threads_base
 * @param attr Mutex attributes object to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_destroy.html
 */
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

#if defined(_POSIX_THREAD_PRIO_PROTECT) || defined(__DOXYGEN__)
/**
 * @brief Get the priority ceiling attribute of a mutex attributes object.
 * @ingroup posix_option_thread_prio_protect
 * @param attr        Mutex attributes object.
 * @param prioceiling Output: priority ceiling.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_getprioceiling.html
 */
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *ZRESTRICT attr,
				     int *ZRESTRICT prioceiling);
#endif

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT) ||                  \
	defined(__DOXYGEN__)
/**
 * @brief Get the protocol attribute of a mutex attributes object.
 * @ingroup posix_option_thread_prio_inherit
 * @param attr     Mutex attributes object.
 * @param protocol Output: @c PTHREAD_PRIO_NONE, @c PTHREAD_PRIO_INHERIT,
 *                 or @c PTHREAD_PRIO_PROTECT.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_getprotocol.html
 */
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *ZRESTRICT attr,
				  int *ZRESTRICT protocol);
#endif

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Get the process-shared attribute of a mutex attributes object.
 * @ingroup posix_option_thread_process_shared
 * @param attr    Mutex attributes object.
 * @param pshared Output: @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_getpshared.html
 */
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *ZRESTRICT attr, int *ZRESTRICT pshared);
#endif

/**
 * @brief Get the robustness attribute of a mutex attributes object.
 * @ingroup posix_option_group_robust_mutexes
 * @param attr   Mutex attributes object.
 * @param robust Output: @c PTHREAD_MUTEX_STALLED or @c PTHREAD_MUTEX_ROBUST.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_getrobust.html
 */
int pthread_mutexattr_getrobust(const pthread_mutexattr_t *ZRESTRICT attr, int *ZRESTRICT robust);

/**
 * @brief Get the type attribute of a mutex attributes object.
 * @ingroup posix_option_group_posix_threads_ext
 * @param attr Mutex attributes object.
 * @param type Output: @c PTHREAD_MUTEX_NORMAL, @c PTHREAD_MUTEX_ERRORCHECK,
 *             @c PTHREAD_MUTEX_RECURSIVE, or @c PTHREAD_MUTEX_DEFAULT.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_gettype.html
 */
int pthread_mutexattr_gettype(const pthread_mutexattr_t *ZRESTRICT attr, int *ZRESTRICT type);

/**
 * @brief Initialise a mutex attributes object with default values.
 * @ingroup posix_option_group_threads_base
 * @param attr Mutex attributes object to initialise.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_init.html
 */
int pthread_mutexattr_init(pthread_mutexattr_t *attr);

#if defined(_POSIX_THREAD_PRIO_PROTECT) || defined(__DOXYGEN__)
/**
 * @brief Set the priority ceiling attribute of a mutex attributes object.
 * @ingroup posix_option_thread_prio_protect
 * @param attr        Mutex attributes object.
 * @param prioceiling Priority ceiling value.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_setprioceiling.html
 */
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling);
#endif

#if defined(_POSIX_THREAD_PRIO_INHERIT) || defined(_POSIX_THREAD_PRIO_PROTECT) ||                  \
	defined(__DOXYGEN__)
/**
 * @brief Set the protocol attribute of a mutex attributes object.
 * @ingroup posix_option_thread_prio_inherit
 * @param attr     Mutex attributes object.
 * @param protocol @c PTHREAD_PRIO_NONE, @c PTHREAD_PRIO_INHERIT, or @c PTHREAD_PRIO_PROTECT.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_setprotocol.html
 */
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol);
#endif

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Set the process-shared attribute of a mutex attributes object.
 * @ingroup posix_option_thread_process_shared
 * @param attr    Mutex attributes object.
 * @param pshared @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_setpshared.html
 */
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
#endif

/**
 * @brief Set the robustness attribute of a mutex attributes object.
 * @ingroup posix_option_group_robust_mutexes
 * @param attr   Mutex attributes object.
 * @param robust @c PTHREAD_MUTEX_STALLED or @c PTHREAD_MUTEX_ROBUST.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_setrobust.html
 */
int pthread_mutexattr_setrobust(pthread_mutexattr_t *attr, int robust);

/**
 * @brief Set the type attribute of a mutex attributes object.
 * @ingroup posix_option_group_posix_threads_ext
 * @param attr Mutex attributes object.
 * @param type @c PTHREAD_MUTEX_NORMAL, @c PTHREAD_MUTEX_ERRORCHECK,
 *             @c PTHREAD_MUTEX_RECURSIVE, or @c PTHREAD_MUTEX_DEFAULT.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutexattr_settype.html
 */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);



/**
 * @brief Ensure a one-time initialisation routine is called exactly once.
 * @ingroup posix_option_group_threads_base
 * @param once_control Pointer to a @c pthread_once_t initialised with @c PTHREAD_ONCE_INIT.
 * @param init_routine Initialisation function to call at most once.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_once.html
 */
int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));



/**
 * @brief Destroy a reader-writer lock.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock Reader-writer lock to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_destroy.html
 */
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);

/**
 * @brief Initialise a reader-writer lock.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock Reader-writer lock to initialise.
 * @param attr   Attributes, or NULL for defaults.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_init.html
 */
int pthread_rwlock_init(pthread_rwlock_t *ZRESTRICT rwlock,
			const pthread_rwlockattr_t *ZRESTRICT attr);

/**
 * @brief Acquire a read lock, blocking until available.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock Reader-writer lock.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_rdlock.html
 */
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);

#if defined(_POSIX_TIMEOUTS) || defined(__DOXYGEN__)
/**
 * @brief Acquire a read lock with an absolute timeout.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock  Reader-writer lock.
 * @param abstime Absolute timeout (CLOCK_REALTIME).
 * @return 0 on success, @c ETIMEDOUT on timeout, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_timedrdlock.html
 */
int pthread_rwlock_timedrdlock(pthread_rwlock_t *ZRESTRICT rwlock,
			       const struct timespec *ZRESTRICT abstime);
#endif

#if defined(_POSIX_TIMEOUTS) || defined(__DOXYGEN__)
/**
 * @brief Acquire a write lock with an absolute timeout.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock  Reader-writer lock.
 * @param abstime Absolute timeout (CLOCK_REALTIME).
 * @return 0 on success, @c ETIMEDOUT on timeout, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_timedwrlock.html
 */
int pthread_rwlock_timedwrlock(pthread_rwlock_t *ZRESTRICT rwlock,
			       const struct timespec *ZRESTRICT abstime);
#endif

/**
 * @brief Try to acquire a read lock without blocking.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock Reader-writer lock.
 * @return 0 on success, @c EBUSY if locked, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_tryrdlock.html
 */
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);

/**
 * @brief Try to acquire a write lock without blocking.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock Reader-writer lock.
 * @return 0 on success, @c EBUSY if locked, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_trywrlock.html
 */
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);

/**
 * @brief Release a reader-writer lock.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock Reader-writer lock to release.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_unlock.html
 */
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);

/**
 * @brief Acquire a write lock, blocking until available.
 * @ingroup posix_option_group_rw_locks
 * @param rwlock Reader-writer lock.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlock_wrlock.html
 */
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);

/**
 * @brief Destroy a reader-writer lock attributes object.
 * @ingroup posix_option_group_rw_locks
 * @param attr Attributes object to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlockattr_destroy.html
 */
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr);

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Get the process-shared attribute of a reader-writer lock attributes object.
 * @ingroup posix_option_group_rw_locks
 * @param attr    Attributes object.
 * @param pshared Output: @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlockattr_getpshared.html
 */
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *ZRESTRICT attr,
				  int *ZRESTRICT pshared);
#endif

/**
 * @brief Initialise a reader-writer lock attributes object with default values.
 * @ingroup posix_option_group_rw_locks
 * @param attr Attributes object to initialise.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlockattr_init.html
 */
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr);

#if defined(_POSIX_THREAD_PROCESS_SHARED) || defined(__DOXYGEN__)
/**
 * @brief Set the process-shared attribute of a reader-writer lock attributes object.
 * @ingroup posix_option_group_rw_locks
 * @param attr    Attributes object.
 * @param pshared @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_rwlockattr_setpshared.html
 */
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared);
#endif



/**
 * @brief Return the thread ID of the calling thread.
 * @ingroup posix_option_group_threads_base
 * @return Thread ID of the calling thread.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_self.html
 */
pthread_t pthread_self(void);

/**
 * @brief Set the cancellability state of the calling thread.
 * @ingroup posix_option_group_threads_base
 * @param state    @c PTHREAD_CANCEL_ENABLE or @c PTHREAD_CANCEL_DISABLE.
 * @param oldstate Output: previous state, or NULL.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setcancelstate.html
 */
int pthread_setcancelstate(int state, int *oldstate);

/**
 * @brief Set the cancellability type of the calling thread.
 * @ingroup posix_option_group_threads_base
 * @param type    @c PTHREAD_CANCEL_DEFERRED or @c PTHREAD_CANCEL_ASYNCHRONOUS.
 * @param oldtype Output: previous type, or NULL.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setcanceltype.html
 */
int pthread_setcanceltype(int type, int *oldtype);

#if (_XOPEN_SOURCE < 800) || defined(__DOXYGEN__)
/**
 * @brief Set the concurrency level (advisory, has no effect on scheduling).
 * @ingroup posix_option_group_xsi_threads_ext
 * @param new_level Desired concurrency level (0 = implementation default).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setconcurrency.html
 */
int pthread_setconcurrency(int new_level);
#endif

#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) || defined(__DOXYGEN__)
/**
 * @brief Set the scheduling policy and parameters of a thread.
 * @ingroup posix_option_thread_priority_scheduling
 * @param thread Thread to modify.
 * @param policy New scheduling policy.
 * @param param  New scheduling parameters.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setschedparam.html
 */
int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param);

/**
 * @brief Set the scheduling priority of a thread.
 * @ingroup posix_option_thread_priority_scheduling
 * @param thread Thread to modify.
 * @param prio   New priority value.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setschedprio.html
 */
int pthread_setschedprio(pthread_t thread, int prio);
#endif

/**
 * @brief Set the thread-specific value associated with a key.
 * @ingroup posix_option_group_threads_base
 * @param key   Thread-specific data key.
 * @param value Value to associate with @p key for the calling thread.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setspecific.html
 */
int pthread_setspecific(pthread_key_t key, const void *value);



/**
 * @brief Destroy a spin lock.
 * @ingroup posix_option_group_spin_locks
 * @param lock Spin lock to destroy.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_spin_destroy.html
 */
int pthread_spin_destroy(pthread_spinlock_t *lock);

/**
 * @brief Initialise a spin lock.
 * @ingroup posix_option_group_spin_locks
 * @param lock    Spin lock to initialise.
 * @param pshared @c PTHREAD_PROCESS_SHARED or @c PTHREAD_PROCESS_PRIVATE.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_spin_init.html
 */
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);

/**
 * @brief Acquire a spin lock, busy-waiting until available.
 * @ingroup posix_option_group_spin_locks
 * @param lock Spin lock to acquire.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_spin_lock.html
 */
int pthread_spin_lock(pthread_spinlock_t *lock);

/**
 * @brief Try to acquire a spin lock without busy-waiting.
 * @ingroup posix_option_group_spin_locks
 * @param lock Spin lock to try.
 * @return 0 on success, @c EBUSY if already locked, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_spin_trylock.html
 */
int pthread_spin_trylock(pthread_spinlock_t *lock);

/**
 * @brief Release a spin lock.
 * @ingroup posix_option_group_spin_locks
 * @param lock Spin lock to release.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_spin_unlock.html
 */
int pthread_spin_unlock(pthread_spinlock_t *lock);



/**
 * @brief Create a cancellation point in the calling thread.
 * @ingroup posix_option_group_threads_base
 *
 * If cancellation is pending and enabled, this function does not return.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_testcancel.html
 */
void pthread_testcancel(void);

/**
 * @brief Push a cleanup handler onto the calling thread's cleanup stack.
 * @ingroup posix_option_group_threads_base
 *
 * The handler @p _rtn is called with @p _arg when the thread exits or calls
 * pthread_cleanup_pop() with a non-zero argument.
 *
 * @note This macro must be paired with pthread_cleanup_pop() in the same
 *       lexical scope.
 */
#define pthread_cleanup_push(_rtn, _arg)                                                           \
	do /* enforce '{'-like behaviour */ {                                                      \
		void *_z_pthread_cleanup[3];                                                       \
	k_thread_cleanup_push(_z_pthread_cleanup, _rtn, _arg)

/**
 * @brief Pop a cleanup handler from the calling thread's cleanup stack.
 * @ingroup posix_option_group_threads_base
 *
 * @param _ex If non-zero, the handler is executed before being removed.
 *
 * @note This macro must be paired with pthread_cleanup_push() in the same
 *       lexical scope.
 */
#define pthread_cleanup_pop(_ex)                                                                   \
	k_thread_cleanup_pop(_ex);                                                                 \
	} /* enforce '}'-like behaviour */                                                         \
	while (0)



#if defined(_GNU_SOURCE) || defined(__DOXYGEN__)

/**
 * @brief Get the name of a thread (GNU extension).
 * @ingroup posix_option_group_non_portable
 * @param thread Thread to query.
 * @param name   Output buffer for the name.
 * @param len    Size of @p name buffer.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getname_np.html
 */
int pthread_getname_np(pthread_t thread, char *name, size_t len);

/**
 * @brief Set the name of a thread (GNU extension).
 * @ingroup posix_option_group_non_portable
 * @param thread Thread to name.
 * @param name   Null-terminated name string (max 15 characters + NUL).
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_setname_np.html
 */
int pthread_setname_np(pthread_t thread, const char *name);

/**
 * @brief Wait for a thread to terminate with an absolute timeout (GNU extension).
 * @ingroup posix_option_group_non_portable
 * @param thread  Thread to wait for.
 * @param status  Output: exit value or @c PTHREAD_CANCELED.
 * @param abstime Absolute timeout (CLOCK_REALTIME).
 * @return 0 on success, @c ETIMEDOUT on timeout, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_timedjoin_np.html
 */
int pthread_timedjoin_np(pthread_t thread, void **status, const struct timespec *abstime);

/**
 * @brief Try to join a thread without blocking (GNU extension).
 * @ingroup posix_option_group_non_portable
 * @param thread Thread to try to join.
 * @param status Output: exit value if joined, or unchanged if not yet terminated.
 * @return 0 if joined, @c EBUSY if the thread has not yet terminated,
 *         or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_tryjoin_np.html
 */
int pthread_tryjoin_np(pthread_t thread, void **status);

#endif /* _GNU_SOURCE || __DOXYGEN__ */


#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_POSIX_PTHREAD_H_ */

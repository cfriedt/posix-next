/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX semaphores API (<semaphore.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/semaphore.h.html">
 *      POSIX.1-2017 &lt;semaphore.h&gt;</a>
 *
 * @ingroup posix_option_group_semaphores
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SEMAPHORE_H_
#define ZEPHYR_INCLUDE_POSIX_SEMAPHORE_H_

#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Returned by sem_open() on failure.  @ingroup posix_option_group_semaphores*/
#define SEM_FAILED ((sem_t *) 0)

#if !(defined(_SEM_T_DECLARED) || defined(__sem_t_defined)) || defined(__DOXYGEN__)
/** @brief Semaphore object type.  @ingroup posix_option_group_semaphores*/
typedef struct k_sem sem_t;
#define _SEM_T_DECLARED
#define __sem_t_defined
#endif

/**
 * @brief Destroy an unnamed semaphore.
 * @ingroup posix_option_group_semaphores
 * @param semaphore Semaphore to destroy.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_destroy.html
 */
int sem_destroy(sem_t *semaphore);

/**
 * @brief Get the current value of a semaphore.
 * @ingroup posix_option_group_semaphores
 * @param semaphore Semaphore to query.
 * @param value     Output: current count (implementation may report 0 if waiters are present).
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_getvalue.html
 */
int sem_getvalue(sem_t *ZRESTRICT semaphore, int *ZRESTRICT value);

/**
 * @brief Initialise an unnamed semaphore.
 * @ingroup posix_option_group_semaphores
 * @param semaphore Semaphore to initialise.
 * @param pshared   Non-zero if the semaphore is shared between processes.
 * @param value     Initial count (must be <= SEM_VALUE_MAX).
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_init.html
 */
int sem_init(sem_t *semaphore, int pshared, unsigned int value);

/**
 * @brief Unlock a semaphore (increment its count).
 * @ingroup posix_option_group_semaphores
 * @param semaphore Semaphore to post.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_post.html
 */
int sem_post(sem_t *semaphore);

/**
 * @brief Lock a semaphore with an absolute timeout.
 * @ingroup posix_option_group_semaphores
 * @param semaphore Semaphore to wait on.
 * @param abstime   Absolute timeout (CLOCK_REALTIME).
 * @return 0 on success, -1 with @c errno = @c ETIMEDOUT on timeout,
 *         or -1 with errno set on another failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_timedwait.html
 */
int sem_timedwait(sem_t *ZRESTRICT semaphore, struct timespec *ZRESTRICT abstime);

/**
 * @brief Try to lock a semaphore without blocking.
 * @ingroup posix_option_group_semaphores
 * @param semaphore Semaphore to try.
 * @return 0 on success, -1 with @c errno = @c EAGAIN if the count is zero,
 *         or -1 with errno set on another failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_trywait.html
 */
int sem_trywait(sem_t *semaphore);

/**
 * @brief Lock a semaphore, blocking until it becomes available.
 * @ingroup posix_option_group_semaphores
 * @param semaphore Semaphore to wait on.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_wait.html
 */
int sem_wait(sem_t *semaphore);

/**
 * @brief Open or create a named semaphore.
 * @ingroup posix_option_group_semaphores
 * @param name   Semaphore name (implementation-defined path).
 * @param oflags Flags: @c O_CREAT, @c O_EXCL.
 * @param ...    If @c O_CREAT is set: mode_t mode, unsigned int value.
 * @return Pointer to the semaphore on success, or @c SEM_FAILED on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_open.html
 */
sem_t *sem_open(const char *name, int oflags, ...);

/**
 * @brief Remove a named semaphore.
 * @ingroup posix_option_group_semaphores
 * @param name Semaphore name.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_unlink.html
 */
int sem_unlink(const char *name);

/**
 * @brief Close a named semaphore.
 * @ingroup posix_option_group_semaphores
 * @param sem Semaphore to close.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sem_close.html
 */
int sem_close(sem_t *sem);


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SEMAPHORE_H_ */

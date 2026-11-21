/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX declarations for waiting (<sys/wait.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_wait.h.html">
 *      POSIX.1-2017 &lt;sys/wait.h&gt;</a>
 *
 * @ingroup posix_option_group_multi_process
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_WAIT_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_WAIT_H_

#include <signal.h>
#include <sys/types.h>

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Report status of any continued child process. */
#define WCONTINUED K_PROCESS_WCONTINUED
#endif
/** @brief Do not hang if no status is available; return immediately. */
#define WNOHANG K_PROCESS_WNOHANG
/** @brief Report status of stopped child process. */
#define WUNTRACED K_PROCESS_WUNTRACED

/** @brief Exit status of a child that exited normally. */
#define WEXITSTATUS(wstatus) K_WEXITSTATUS(wstatus)
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief True if the child has continued from a job control stop. */
#define WIFCONTINUED(wstatus) ((wstatus) == 0xffff)
#endif
/** @brief True if the child exited normally. */
#define WIFEXITED(wstatus) K_WIFEXITED(wstatus)
/** @brief True if the child exited due to an uncaught signal. */
#define WIFSIGNALED(wstatus) K_WIFSIGNALED(wstatus)
/** @brief True if the child is currently stopped. */
#define WIFSTOPPED(wstatus) (((wstatus) & 0xff) == 0x7f)
/** @brief Signal number that caused the child to stop. */
#define WSTOPSIG(wstatus) (((wstatus) >> 8) & 0xff)
/** @brief Signal number that caused the child to terminate. */
#define WTERMSIG(wstatus) K_WTERMSIG(wstatus)

/** @brief Wait for processes that have exited. */
#define WEXITED K_PROCESS_WEXITED
/** @brief Report the status of a selected process, leaving it waitable. */
#define WNOWAIT K_PROCESS_WNOWAIT
/** @brief Wait for processes stopped by delivery of a signal. */
#define WSTOPPED K_PROCESS_WUNTRACED

#if !defined(_IDTYPE_T_DECLARED) && !defined(__idtype_t_defined) || defined(__DOXYGEN__)
/** @brief Identifier kind selecting the scope of a waitid() operation. */
typedef enum idtype {
	/** Wait for any child of the calling process. */
	P_ALL,
	/** Wait for the child with a specific process ID. */
	P_PID,
	/** Wait for any child in a specific process group. */
	P_PGID,
} idtype_t;
#define _IDTYPE_T_DECLARED
#define __idtype_t_defined
#endif

#if !defined(_ID_T_DECLARED) && !defined(__id_t_defined)
typedef int id_t;
#define _ID_T_DECLARED
#define __id_t_defined
#endif

#if !defined(_PID_T_DECLARED) && !defined(__pid_t_defined)
typedef int pid_t;
#define _PID_T_DECLARED
#define __pid_t_defined
#endif

/**
 * @brief Wait for any child process to terminate.
 * @param[out] stat_loc location in which to store the child's wait status; may be NULL.
 * @return the process ID of the reaped child on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/wait.html
 */
pid_t wait(int *stat_loc);

/**
 * @brief Wait for a child process matching @a idtype and @a id to change state.
 * @param idtype scope of the wait operation (@ref P_ALL, @ref P_PID, or @ref P_PGID).
 * @param id process ID or process group ID, according to @a idtype.
 * @param[out] infop location in which to store the child's status information.
 * @param options bitwise OR of @ref WEXITED, @ref WNOHANG, and @ref WNOWAIT.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/waitid.html
 */
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);

/**
 * @brief Wait for a child process to terminate.
 * @param pid child selection: a process ID, -1 for any child, 0 for the caller's process
 *            group, or a negated process group ID.
 * @param[out] stat_loc location in which to store the child's wait status; may be NULL.
 * @param options bitwise OR of @ref WNOHANG and @ref WUNTRACED.
 * @return the process ID of the child on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/waitpid.html
 */
pid_t waitpid(pid_t pid, int *stat_loc, int options);

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_WAIT_H_ */

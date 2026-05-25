/*
 * Copyright (c) 2024 Meta Platforms
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX password database access (<pwd.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pwd.h.html">
 *      POSIX.1-2017 &lt;pwd.h&gt;</a>
 *
 * @defgroup posix_pwd POSIX Password Database
 * @ingroup posix_option_thread_safe_functions
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_PWD_H_
#define ZEPHYR_INCLUDE_POSIX_PWD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>

/** @brief Password database entry. */
struct passwd {
	char *pw_name;  /**< User's login name. */
	uid_t pw_uid;   /**< Numerical user ID. */
	gid_t pw_gid;   /**< Numerical group ID. */
	char *pw_dir;   /**< Initial working directory. */
	char *pw_shell; /**< Program to use as shell. */
};

/**
 * @brief Look up a password entry by name (thread-safe).
 * @param nam     User login name to search for.
 * @param pwd     Caller-supplied structure to fill in.
 * @param buffer  Caller-supplied buffer for string fields.
 * @param bufsize Size of @p buffer.
 * @param result  Output: pointer to @p pwd on success, or NULL if not found.
 * @return 0 on success, or a positive error number on failure.
 */
int getpwnam_r(const char *nam, struct passwd *pwd, char *buffer, size_t bufsize,
	       struct passwd **result);

/**
 * @brief Look up a password entry by user ID (thread-safe).
 * @param uid     Numerical user ID to search for.
 * @param pwd     Caller-supplied structure to fill in.
 * @param buffer  Caller-supplied buffer for string fields.
 * @param bufsize Size of @p buffer.
 * @param result  Output: pointer to @p pwd on success, or NULL if not found.
 * @return 0 on success, or a positive error number on failure.
 */
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_PWD_H_ */

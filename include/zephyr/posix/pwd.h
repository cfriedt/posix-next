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
 * @ingroup posix_option_group_system_database_r
 */

#ifndef ZEPHYR_INCLUDE_POSIX_PWD_H_
#define ZEPHYR_INCLUDE_POSIX_PWD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>

#if (!defined(_PASSWD_DECLARED) && !defined(__passwd_defined)) || defined(__DOXYGEN__)
/**
 * @brief POSIX password database entry.
 * @ingroup posix_option_group_system_database
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/pwd.h.html
 */
struct passwd {
	char *pw_name;    /**< User's login name. */
	char *pw_passwd;  /**< Encrypted password. */
	uid_t pw_uid;     /**< Numerical user ID. */
	gid_t pw_gid;     /**< Numerical group ID. */
	char *pw_comment; /**< User comment (unused). */
	char *pw_gecos;   /**< Real name / GECOS field. */
	char *pw_dir;     /**< Initial working directory. */
	char *pw_shell;   /**< Program to use as shell. */
};
#define _PASSWD_DECLARED
#define __passwd_defined
#endif

/**
 * @brief Look up a password entry by name (thread-safe).
 * @ingroup posix_option_group_system_database_r
 * @param nam     User login name to search for.
 * @param pwd     Caller-supplied structure to fill in.
 * @param buffer  Caller-supplied buffer for string fields.
 * @param bufsize Size of @p buffer.
 * @param result  Output: pointer to @p pwd on success, or NULL if not found.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpwnam_r.html
 */
int getpwnam_r(const char *nam, struct passwd *pwd, char *buffer, size_t bufsize,
	       struct passwd **result);

/**
 * @brief Look up a password entry by user ID (thread-safe).
 * @ingroup posix_option_group_system_database_r
 * @param uid     Numerical user ID to search for.
 * @param pwd     Caller-supplied structure to fill in.
 * @param buffer  Caller-supplied buffer for string fields.
 * @param bufsize Size of @p buffer.
 * @param result  Output: pointer to @p pwd on success, or NULL if not found.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpwuid_r.html
 */
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);

#if defined(_XOPEN_SOURCE)
/**
 * @brief Close the password database.
 * @ingroup posix_option_group_system_database
 */
void endpwent(void);

/**
 * @brief Read the next password entry from the password database.
 * @ingroup posix_option_group_system_database
 */
struct passwd *getpwent(void);
#endif

/**
 * @brief Look up a password entry by name.
 * @ingroup posix_option_group_system_database
 * @param name User login name to search for.
 * @return Pointer to a static passwd structure, or NULL on failure or if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpwnam.html
 */
struct passwd *getpwnam(const char *name);

/**
 * @brief Look up a password entry by user ID.
 * @ingroup posix_option_group_system_database
 * @param uid Numerical user ID to search for.
 * @return Pointer to a static passwd structure, or NULL on failure or if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getpwuid.html
 */
struct passwd *getpwuid(uid_t uid);

#if defined(_XOPEN_SOURCE)
/**
 * @brief Rewind to the beginning of the password database.
 * @ingroup posix_option_group_system_database
 */
void setpwent(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_PWD_H_ */

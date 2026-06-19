/*
 * Copyright (c) 2024 Meta Platforms
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX group database access (<grp.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/grp.h.html">
 *      POSIX.1-2017 &lt;grp.h&gt;</a>
 *
 * @ingroup posix_option_group_system_database_r
 */

#ifndef ZEPHYR_INCLUDE_POSIX_GRP_H_
#define ZEPHYR_INCLUDE_POSIX_GRP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>

#if !defined(_GROUP_DECLARED) && !defined(__group_defined)
/**
 * @brief POSIX group database entry.
 * @ingroup posix_option_group_system_database
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/group.h.html
 */
struct group {
	char *gr_name;    /**< Group name. */
	char *gr_passwd;  /**< Group password. */
	gid_t gr_gid;     /**< Numerical group ID. */
	char **gr_mem;    /**< NULL-terminated array of member login names. */
};
#define _GROUP_DECLARED
#define __group_defined
#endif

/**
 * @brief Look up a group entry by name (thread-safe).
 * @ingroup posix_option_group_system_database_r
 * @param name    Group name to search for.
 * @param grp     Caller-supplied structure to fill in.
 * @param buffer  Caller-supplied buffer for string fields.
 * @param bufsize Size of @p buffer.
 * @param result  Output: pointer to @p grp on success, or NULL if not found.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getgrnam_r.html
 */
int getgrnam_r(const char *name, struct group *grp, char *buffer, size_t bufsize,
	       struct group **result);

/**
 * @brief Look up a group entry by group ID (thread-safe).
 * @ingroup posix_option_group_system_database_r
 * @param gid     Numerical group ID to search for.
 * @param grp     Caller-supplied structure to fill in.
 * @param buffer  Caller-supplied buffer for string fields.
 * @param bufsize Size of @p buffer.
 * @param result  Output: pointer to @p grp on success, or NULL if not found.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getgrgid_r.html
 */
int getgrgid_r(gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result);


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_GRP_H_ */

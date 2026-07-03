/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX UNIX domain socket types (<sys/un.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_un.h.html">
 *      POSIX.1-2017 &lt;sys/un.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_UN_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_UN_H_

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum length of a UNIX domain socket pathname (implementation). */
#define UNIX_PATH_MAX 108

#if !(defined(_SOCKADDR_UN_DECLARED) || defined(__sockaddr_un_defined)) || defined(__DOXYGEN__)
/** @brief UNIX domain socket address. */
struct sockaddr_un {
	sa_family_t sun_family;        /**< AF_UNIX. */
	char        sun_path[UNIX_PATH_MAX]; /**< Socket pathname. */
};
#define _SOCKADDR_UN_DECLARED
#define __sockaddr_un_defined
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_UN_H_ */

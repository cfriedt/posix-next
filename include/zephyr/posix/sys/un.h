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

#include <zephyr/net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_SA_FAMILY_T_DECLARED) && !defined(__sa_family_t_defined)
typedef net_sa_family_t sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
#endif

#if !(defined(_SOCKADDR_UN_DECLARED) || defined(__sockaddr_un_defined)) || defined(__DOXYGEN__)
/** @brief UNIX domain socket address. */
struct sockaddr_un {
	sa_family_t sun_family; /**< Address family; @ref AF_UNIX. */
	char        sun_path[NET_SOCKADDR_MAX_SIZE - sizeof(sa_family_t)]; /**< Socket pathname. */
};
#define _SOCKADDR_UN_DECLARED
#define __sockaddr_un_defined
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYS_UN_H_ */

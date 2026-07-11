/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief TCP definitions (<netinet/tcp.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netinet_tcp.h.html">
 *      POSIX.1-2017 &lt;netinet/tcp.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETINET_TCP_H_
#define ZEPHYR_INCLUDE_POSIX_NETINET_TCP_H_

/** @brief Avoid coalescing of small segments. */
#ifndef TCP_NODELAY
#define TCP_NODELAY 1
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_TCP_H_ */

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
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

#include <zephyr/net/socket.h>

/** @brief Disable Nagle's algorithm. */
#define TCP_NODELAY ZSOCK_TCP_NODELAY

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_TCP_H_ */

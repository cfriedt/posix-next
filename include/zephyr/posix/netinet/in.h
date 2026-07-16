/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Internet address types (<netinet/in.h>)
 *
 * Provides the fundamental Internet address and port types used across the
 * BSD socket API and the IP protocol family headers.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netinet_in.h.html">
 *      POSIX.1-2017 &lt;netinet/in.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_
#define ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_

#include <stdint.h>

#include <zephyr/net/socket.h>

/* Temporary workaround required to build with Zephyr 4.4+ */
#define ZEPHYR_INCLUDE_NET_COMPAT_MODE_SYMBOLS
#include <zephyr/net/net_compat.h>
#undef ZEPHYR_INCLUDE_NET_COMPAT_MODE_SYMBOLS

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Unsigned 16-bit Internet port number.  @ingroup posix_option_group_networking*/
typedef uint16_t in_port_t;

/** @brief Unsigned 32-bit IPv4 address.  @ingroup posix_option_group_networking*/
typedef uint32_t in_addr_t;


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_ */

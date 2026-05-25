/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Internet address conversion functions (<arpa/inet.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/arpa_inet.h.html">
 *      POSIX.1-2017 &lt;arpa/inet.h&gt;</a>
 *
 * @defgroup posix_arpa_inet Internet Address Conversions
 * @ingroup posix_option_group_networking
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_ARPA_INET_H_
#define ZEPHYR_INCLUDE_POSIX_ARPA_INET_H_

#include <stddef.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <zephyr/net/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Unsigned 32-bit IPv4 address (alias for in_addr_t). */
typedef uint32_t in_addr_t;

/**
 * @brief Convert an IPv4 address from dotted-decimal text to binary.
 *
 * @note Deprecated; use inet_pton() for new code.
 *
 * @param cp Dotted-decimal IPv4 address string (e.g. "192.0.2.1").
 * @return IPv4 address in network byte order, or INADDR_NONE on failure.
 */
in_addr_t inet_addr(const char *cp);

/**
 * @brief Convert an IPv4 address from binary to dotted-decimal text.
 *
 * @note Deprecated; use inet_ntop() for new code.  The returned pointer
 *       is to a static buffer that may be overwritten by subsequent calls.
 *
 * @param in IPv4 address in network byte order.
 * @return Pointer to a dotted-decimal string, or NULL on failure.
 */
char *inet_ntoa(struct in_addr in);

/**
 * @brief Convert an IPv4 or IPv6 address from binary to text form.
 * @param family Address family: AF_INET or AF_INET6.
 * @param src    Source address in network byte order.
 * @param dst    Output buffer for the text form.
 * @param size   Size of @p dst in bytes (INET_ADDRSTRLEN / INET6_ADDRSTRLEN).
 * @return @p dst on success, or NULL with errno set on failure.
 */
char *inet_ntop(sa_family_t family, const void *src, char *dst, size_t size);

/**
 * @brief Convert an IPv4 or IPv6 address from text form to binary.
 * @param family Address family: AF_INET or AF_INET6.
 * @param src    Text form of the address.
 * @param dst    Output buffer for the binary address.
 * @return 1 on success, 0 if @p src is not a valid address, -1 on error.
 */
int inet_pton(sa_family_t family, const char *src, void *dst);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_ARPA_INET_H_ */

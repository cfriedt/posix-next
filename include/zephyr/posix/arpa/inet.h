/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Internet address conversion functions (<arpa/inet.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/arpa_inet.h.html">
 *      POSIX.1-2017 &lt;arpa/inet.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_ARPA_INET_H_
#define ZEPHYR_INCLUDE_POSIX_ARPA_INET_H_

#include <stddef.h>
#include <stdint.h>

#include <zephyr/net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_SA_FAMILY_T_DECLARED) && !defined(__sa_family_t_defined)
typedef net_sa_family_t sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
#endif

#if !defined(_IN_PORT_T_DECLARED) && !defined(__in_port_t_defined)
typedef uint16_t in_port_t;
#define _IN_PORT_T_DECLARED
#define __in_port_t_defined
#endif

#if !defined(_IN_ADDR_T_DECLARED) && !defined(__in_addr_t_defined)
typedef uint32_t in_addr_t;
#define _IN_ADDR_T_DECLARED
#define __in_addr_t_defined
#endif

#if !defined(_IN_ADDR_DECLARED) && !defined(__in_addr_defined)
struct in_addr {
	in_addr_t s_addr;
};
#define _IN_ADDR_DECLARED
#define __in_addr_defined
#endif

#define INET_ADDRSTRLEN  NET_INET_ADDRSTRLEN
#define INET6_ADDRSTRLEN NET_INET6_ADDRSTRLEN

/** @brief Convert 32-bit value from host to network byte order. */
#define htonl(x) net_htonl(x)
/** @brief Convert 16-bit value from host to network byte order. */
#define htons(x) net_htons(x)
/** @brief Convert 32-bit value from network to host byte order. */
#define ntohl(x) net_ntohl(x)
/** @brief Convert 16-bit value from network to host byte order. */
#define ntohs(x) net_ntohs(x)

/**
 * @brief Convert an IPv4 address from dotted-decimal text to binary.
 *
 * @note Deprecated; use inet_pton() for new code.
 *
 * @param cp Dotted-decimal IPv4 address string (e.g. "192.0.2.1").
 * @return IPv4 address in network byte order, or `(in_addr_t)(-1)` on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_addr.html
 */
in_addr_t inet_addr(const char *cp);

/**
 * @brief Convert an IPv4 address from binary to dotted-decimal text.
 *
 * @note Deprecated; use inet_ntop() for new code.  The returned pointer
 *       is to a static buffer that may be overwritten by subsequent calls.
 *
 * @param in IPv4 address in network byte order.
 * @return Pointer to a dotted-decimal string, or @c NULL on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_ntoa.html
 */
char *inet_ntoa(struct in_addr in);

/**
 * @brief Convert an IPv4 or IPv6 address from binary to text form.
 * @param family Address family: @ref AF_INET or @ref AF_INET6.
 * @param src    Source address in network byte order.
 * @param dst    Output buffer for the text form.
 * @param size   Size of @p dst in bytes (@c INET_ADDRSTRLEN / @c INET6_ADDRSTRLEN).
 * @return @p dst on success, or @c NULL with errno set on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_ntop.html
 */
char *inet_ntop(sa_family_t family, const void *src, char *dst, size_t size);

/**
 * @brief Convert an IPv4 or IPv6 address from text form to binary.
 * @param family Address family: @ref AF_INET or @ref AF_INET6.
 * @param src    Text form of the address.
 * @param dst    Output buffer for the binary address.
 * @return 1 on success, 0 if @p src is not a valid address, or -1 with errno set on error.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_pton.html
 */
int inet_pton(sa_family_t family, const char *src, void *dst);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_ARPA_INET_H_ */

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

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Error return from inet_addr() for invalid input. */
#define INADDR_NONE ((in_addr_t)0xffffffff)

/** @brief Convert 32-bit value from host to network byte order. */
#if defined(__GNUC__) || defined(__clang__)
#define htonl(x) __builtin_bswap32((uint32_t)(x))
#define htons(x) __builtin_bswap16((uint16_t)(x))
#define ntohl(x) __builtin_bswap32((uint32_t)(x))
#define ntohs(x) __builtin_bswap16((uint16_t)(x))
#else
#define htonl(x) ((uint32_t)((((uint32_t)(x) & 0x000000ffU) << 24) | \
			     (((uint32_t)(x) & 0x0000ff00U) << 8) |  \
			     (((uint32_t)(x) & 0x00ff0000U) >> 8) |  \
			     (((uint32_t)(x) & 0xff000000U) >> 24)))
#define htons(x) ((uint16_t)((((uint16_t)(x) & 0x00ffU) << 8) | \
			     (((uint16_t)(x) & 0xff00U) >> 8)))
#define ntohl(x) htonl(x)
#define ntohs(x) htons(x)
#endif

/**
 * @brief Convert an IPv4 address from dotted-decimal text to binary.
 *
 * @note Deprecated; use inet_pton() for new code.
 *
 * @param cp Dotted-decimal IPv4 address string (e.g. "192.0.2.1").
 * @return IPv4 address in network byte order, or INADDR_NONE on failure.
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
 * @return Pointer to a dotted-decimal string, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_ntoa.html
 */
char *inet_ntoa(struct in_addr in);

/**
 * @brief Convert an IPv4 or IPv6 address from binary to text form.
 * @param family Address family: AF_INET or AF_INET6.
 * @param src    Source address in network byte order.
 * @param dst    Output buffer for the text form.
 * @param size   Size of @p dst in bytes (INET_ADDRSTRLEN / INET6_ADDRSTRLEN).
 * @return @p dst on success, or NULL with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_ntop.html
 */
char *inet_ntop(sa_family_t family, const void *src, char *dst, size_t size);

/**
 * @brief Convert an IPv4 or IPv6 address from text form to binary.
 * @param family Address family: AF_INET or AF_INET6.
 * @param src    Text form of the address.
 * @param dst    Output buffer for the binary address.
 * @return 1 on success, 0 if @p src is not a valid address, -1 on error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/inet_pton.html
 */
int inet_pton(sa_family_t family, const char *src, void *dst);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_ARPA_INET_H_ */

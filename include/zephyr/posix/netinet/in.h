/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Internet address types and constants (<netinet/in.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netinet_in.h.html">
 *      POSIX.1-2017 &lt;netinet/in.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_
#define ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_

#include <stdint.h>

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_IN_ADDR_T_DECLARED) || defined(__in_addr_t_defined)) || defined(__DOXYGEN__)
/** @brief Unsigned 32-bit IPv4 address. */
typedef uint32_t in_addr_t;
#define _IN_ADDR_T_DECLARED
#define __in_addr_t_defined
#endif

#if !(defined(_IN_PORT_T_DECLARED) || defined(__in_port_t_defined)) || defined(__DOXYGEN__)
/** @brief Unsigned 16-bit Internet port number. */
typedef uint16_t in_port_t;
#define _IN_PORT_T_DECLARED
#define __in_port_t_defined
#endif

#if !defined(ZEPHYR_INCLUDE_NET_NET_IP_H_)
#if !(defined(_IN_ADDR_DECLARED) || defined(__in_addr_defined)) || defined(__DOXYGEN__)
/** @brief IPv4 address. */
struct in_addr {
	union {
		uint8_t  s4_addr[4];
		uint16_t s4_addr16[2];
		uint32_t s4_addr32[1];
		in_addr_t s_addr;
	};
};
#define _IN_ADDR_DECLARED
#define __in_addr_defined
#endif
#else
#ifndef _IN_ADDR_DECLARED
#define _IN_ADDR_DECLARED
#define __in_addr_defined
#endif
#endif /* !ZEPHYR_INCLUDE_NET_NET_IP_H_ */

#if !(defined(_SOCKADDR_IN_DECLARED) || defined(__sockaddr_in_defined)) || defined(__DOXYGEN__)
/** @brief Socket address for IPv4. */
struct sockaddr_in {
	sa_family_t    sin_family; /**< AF_INET. */
	in_port_t      sin_port;   /**< Port number in network byte order. */
	struct in_addr sin_addr;   /**< IPv4 address. */
	char           sin_zero[8];
};
#define _SOCKADDR_IN_DECLARED
#define __sockaddr_in_defined
#endif

#if !defined(ZEPHYR_INCLUDE_NET_NET_IP_H_)
#if !(defined(_IN6_ADDR_DECLARED) || defined(__in6_addr_defined)) || defined(__DOXYGEN__)
/** @brief IPv6 address. */
struct in6_addr {
	union {
		uint8_t  s6_addr[16];
		uint16_t s6_addr16[8];
		uint32_t s6_addr32[4];
	};
};
#define _IN6_ADDR_DECLARED
#define __in6_addr_defined
#endif
#else
#ifndef _IN6_ADDR_DECLARED
#define _IN6_ADDR_DECLARED
#define __in6_addr_defined
#endif
#endif /* !ZEPHYR_INCLUDE_NET_NET_IP_H_ */

#if !(defined(_SOCKADDR_IN6_DECLARED) || defined(__sockaddr_in6_defined)) || defined(__DOXYGEN__)
/** @brief Socket address for IPv6. */
struct sockaddr_in6 {
	sa_family_t     sin6_family;   /**< AF_INET6. */
	in_port_t       sin6_port;     /**< Port number in network byte order. */
	uint32_t        sin6_flowinfo; /**< IPv6 flow information. */
	struct in6_addr sin6_addr;     /**< IPv6 address. */
	uint32_t        sin6_scope_id; /**< Scope identifier. */
};
#define _SOCKADDR_IN6_DECLARED
#define __sockaddr_in6_defined
#endif

#if !defined(ZEPHYR_INCLUDE_NET_NET_IP_H_)
#if !(defined(_IPV6_MREQ_DECLARED) || defined(__ipv6_mreq_defined)) || defined(__DOXYGEN__)
/** @brief IPv6 multicast group request. */
struct ipv6_mreq {
	struct in6_addr ipv6mr_multiaddr; /**< Multicast address. */
	unsigned int    ipv6mr_interface; /**< Interface index. */
};
#define _IPV6_MREQ_DECLARED
#define __ipv6_mreq_defined
#endif
#else
#ifndef _IPV6_MREQ_DECLARED
#define _IPV6_MREQ_DECLARED
#define __ipv6_mreq_defined
#endif
#endif /* !ZEPHYR_INCLUDE_NET_NET_IP_H_ */

/** @brief IP pseudo-protocol for socket-level options. */
#define IPPROTO_IP ZSOCK_IPPROTO_IP

/** @brief Internet Control Message Protocol. */
#define IPPROTO_ICMP ZSOCK_IPPROTO_ICMP

/** @brief Internet Group Management Protocol. */
#define IPPROTO_IGMP ZSOCK_IPPROTO_IGMP

/** @brief Transmission Control Protocol. */
#define IPPROTO_TCP ZSOCK_IPPROTO_TCP

/** @brief User Datagram Protocol. */
#define IPPROTO_UDP ZSOCK_IPPROTO_UDP

/** @brief IPv6 header. */
#define IPPROTO_IPV6 ZSOCK_IPPROTO_IPV6

/** @brief Internet Control Message Protocol for IPv6. */
#define IPPROTO_ICMPV6 ZSOCK_IPPROTO_ICMPV6

/** @brief Raw IP packets. */
#define IPPROTO_RAW ZSOCK_IPPROTO_RAW

/** @brief Restrict IPv6 socket to IPv6 traffic only. */
#define IPV6_V6ONLY ZSOCK_IPV6_V6ONLY

/** @brief Set the unicast hop limit for the socket. */
#define IPV6_UNICAST_HOPS ZSOCK_IPV6_UNICAST_HOPS

/** @brief Set multicast output network interface index for the socket. */
#define IPV6_MULTICAST_IF ZSOCK_IPV6_MULTICAST_IF

/** @brief Set the multicast hop limit for the socket. */
#define IPV6_MULTICAST_HOPS ZSOCK_IPV6_MULTICAST_HOPS

/** @brief Set the multicast loop bit for the socket. */
#define IPV6_MULTICAST_LOOP ZSOCK_IPV6_MULTICAST_LOOP

/** @brief Join IPv6 multicast group. */
#define IPV6_ADD_MEMBERSHIP ZSOCK_IPV6_ADD_MEMBERSHIP

/** @brief Leave IPv6 multicast group. */
#define IPV6_DROP_MEMBERSHIP ZSOCK_IPV6_DROP_MEMBERSHIP

/** @brief Join IPv6 multicast group. */
#define IPV6_JOIN_GROUP ZSOCK_IPV6_JOIN_GROUP

/** @brief Leave IPv6 multicast group. */
#define IPV6_LEAVE_GROUP ZSOCK_IPV6_LEAVE_GROUP

/** @brief IPv4 wildcard address (0.0.0.0). */
#define INADDR_ANY ZSOCK_INADDR_ANY

/** @brief IPv4 limited broadcast address (255.255.255.255). */
#define INADDR_BROADCAST ZSOCK_INADDR_BROADCAST

/** @brief IPv4 loopback address (127.0.0.1). */
#define INADDR_LOOPBACK ZSOCK_INADDR_LOOPBACK

/** @brief IPv4 wildcard address initializer. */
#define INADDR_ANY_INIT ZSOCK_INADDR_ANY_INIT

/** @brief IPv4 loopback address initializer. */
#define INADDR_LOOPBACK_INIT ZSOCK_INADDR_LOOPBACK_INIT

#if !defined(ZEPHYR_INCLUDE_NET_NET_IP_H_)
/** @brief IPv6 wildcard address. */
extern const struct in6_addr in6addr_any;

/** @brief IPv6 loopback address. */
extern const struct in6_addr in6addr_loopback;
#endif /* !ZEPHYR_INCLUDE_NET_NET_IP_H_ */


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_ */

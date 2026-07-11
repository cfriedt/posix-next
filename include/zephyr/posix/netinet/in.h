/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Internet address family (<netinet/in.h>)
 *
 * Provides the Internet address and port types, the IPv4/IPv6 socket address
 * structures, the IPv6 wildcard/loopback address objects, the IP protocol and
 * IPv6 socket-option constants, and the IPv6 address-classification macros.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netinet_in.h.html">
 *      POSIX.1-2017 &lt;netinet/in.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_
#define ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_

#include <stdint.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_IN_PORT_T_DECLARED) || defined(__in_port_t_defined)) || defined(__DOXYGEN__)
/** @brief Unsigned 16-bit Internet port number. */
typedef uint16_t in_port_t;
#define _IN_PORT_T_DECLARED
#define __in_port_t_defined
#endif

#if !(defined(_IN_ADDR_T_DECLARED) || defined(__in_addr_t_defined)) || defined(__DOXYGEN__)
/** @brief Unsigned 32-bit IPv4 address. */
typedef uint32_t in_addr_t;
#define _IN_ADDR_T_DECLARED
#define __in_addr_t_defined
#endif

#if !defined(_SA_FAMILY_T_DECLARED) && !defined(__sa_family_t_defined)
typedef uint16_t sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
#endif

#if !(defined(_IN_ADDR_DECLARED) || defined(__in_addr_defined)) || defined(__DOXYGEN__)
/** @brief IPv4 address. */
struct in_addr {
	/** @brief IPv4 address in network byte order. */
	in_addr_t s_addr;
};
#define _IN_ADDR_DECLARED
#define __in_addr_defined
#endif

/* uint16_t and uint32_t are provided by <stdint.h> */

#if !(defined(_SOCKADDR_IN_DECLARED) || defined(__sockaddr_in_defined)) || defined(__DOXYGEN__)
/** @brief IPv4 socket address. */
struct sockaddr_in {
	/** @brief Address family (@c AF_INET). */
	sa_family_t sin_family;
	/** @brief Port number in network byte order. */
	in_port_t sin_port;
	/** @brief IPv4 address in network byte order. */
	struct in_addr sin_addr;
	/** @brief Padding to the size of @c struct sockaddr. */
	uint8_t sin_zero[8];
};
#define _SOCKADDR_IN_DECLARED
#define __sockaddr_in_defined
#endif

#if !(defined(_IN6_ADDR_DECLARED) || defined(__in6_addr_defined)) || defined(__DOXYGEN__)
/** @brief IPv6 address. */
struct in6_addr {
	/** @brief 128-bit IPv6 address in network byte order. */
	uint8_t s6_addr[16];
};
#define _IN6_ADDR_DECLARED
#define __in6_addr_defined
#endif

#if !(defined(_SOCKADDR_IN6_DECLARED) || defined(__sockaddr_in6_defined)) || defined(__DOXYGEN__)
/** @brief IPv6 socket address. */
struct sockaddr_in6 {
	/** @brief Address family (@c AF_INET6). */
	sa_family_t sin6_family;
	/** @brief Port number in network byte order. */
	in_port_t sin6_port;
	/** @brief IPv6 traffic class and flow information. */
	uint32_t sin6_flowinfo;
	/** @brief IPv6 address in network byte order. */
	struct in6_addr sin6_addr;
	/** @brief Set of interfaces for a scope. */
	uint32_t sin6_scope_id;
};
#define _SOCKADDR_IN6_DECLARED
#define __sockaddr_in6_defined
#endif

/** @brief Initializer for a @c struct in6_addr holding the IPv6 wildcard address. */
#define IN6ADDR_ANY_INIT {{0}}

/** @brief Initializer for a @c struct in6_addr holding the IPv6 loopback address. */
#define IN6ADDR_LOOPBACK_INIT {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}}

/** @brief IPv6 wildcard address. */
extern const struct in6_addr in6addr_any;

/** @brief IPv6 loopback address. */
extern const struct in6_addr in6addr_loopback;

#if !(defined(_IPV6_MREQ_DECLARED) || defined(__ipv6_mreq_defined)) || defined(__DOXYGEN__)
/** @brief IPv6 multicast group membership request. */
struct ipv6_mreq {
	/** @brief IPv6 multicast address. */
	struct in6_addr ipv6mr_multiaddr;
	/** @brief Interface index. */
	uint32_t ipv6mr_interface;
};
#define _IPV6_MREQ_DECLARED
#define __ipv6_mreq_defined
#endif

/** @brief Internet protocol. */
#define IPPROTO_IP     ZSOCK_IPPROTO_IP
/** @brief Internet Protocol Version 6. */
#define IPPROTO_IPV6   ZSOCK_IPPROTO_IPV6
/** @brief Internet Control Message Protocol. */
#define IPPROTO_ICMP   ZSOCK_IPPROTO_ICMP
/** @brief ICMP for IPv6. */
#define IPPROTO_ICMPV6 ZSOCK_IPPROTO_ICMPV6
/** @brief Raw IP Packets Protocol. */
#define IPPROTO_RAW    ZSOCK_IPPROTO_RAW
/** @brief Transmission Control Protocol. */
#define IPPROTO_TCP    ZSOCK_IPPROTO_TCP
/** @brief User Datagram Protocol. */
#define IPPROTO_UDP    ZSOCK_IPPROTO_UDP

/** @brief IPv4 wildcard address. */
#define INADDR_ANY       ((in_addr_t)ZSOCK_INADDR_ANY)
/** @brief IPv4 broadcast address. */
#define INADDR_BROADCAST ((in_addr_t)ZSOCK_INADDR_BROADCAST)

#if !defined(INET_ADDRSTRLEN) || defined(__DOXYGEN__)
/** @brief Length of the string form of an IPv4 address. */
#define INET_ADDRSTRLEN ZSOCK_INET_ADDRSTRLEN
#endif

#if !defined(INET6_ADDRSTRLEN) || defined(__DOXYGEN__)
/** @brief Length of the string form of an IPv6 address. */
#define INET6_ADDRSTRLEN ZSOCK_INET6_ADDRSTRLEN
#endif

#if !(defined(htonl) && defined(htons) && defined(ntohl) && defined(ntohs))
#define htonl(x) zsock_htonl(x)
#define htons(x) zsock_htons(x)
#define ntohl(x) zsock_ntohl(x)
#define ntohs(x) zsock_ntohs(x)
#endif

/** @brief Join a multicast group. */
#define IPV6_JOIN_GROUP     ZSOCK_IPV6_JOIN_GROUP
/** @brief Quit a multicast group. */
#define IPV6_LEAVE_GROUP    ZSOCK_IPV6_LEAVE_GROUP
/** @brief Multicast hop limit. */
#define IPV6_MULTICAST_HOPS ZSOCK_IPV6_MULTICAST_HOPS
/** @brief Interface to use for outgoing multicast packets. */
#define IPV6_MULTICAST_IF   ZSOCK_IPV6_MULTICAST_IF
/** @brief Multicast packets are delivered back to the local application. */
#define IPV6_MULTICAST_LOOP ZSOCK_IPV6_MULTICAST_LOOP
/** @brief Unicast hop limit. */
#define IPV6_UNICAST_HOPS   ZSOCK_IPV6_UNICAST_HOPS
/** @brief Restrict an AF_INET6 socket to IPv6 communications only. */
#define IPV6_V6ONLY         ZSOCK_IPV6_V6ONLY

/** @brief Test for the unspecified IPv6 address. */
#define IN6_IS_ADDR_UNSPECIFIED(a) ZSOCK_IN6_IS_ADDR_UNSPECIFIED((const struct zsock_in6_addr *)(a))

/** @brief Test for the IPv6 loopback address. */
#define IN6_IS_ADDR_LOOPBACK(a) ZSOCK_IN6_IS_ADDR_LOOPBACK((const struct zsock_in6_addr *)(a))

/** @brief Test for an IPv6 multicast address. */
#define IN6_IS_ADDR_MULTICAST(a) ZSOCK_IN6_IS_ADDR_MULTICAST((const struct zsock_in6_addr *)(a))

/** @brief Test for a unicast link-local IPv6 address. */
#define IN6_IS_ADDR_LINKLOCAL(a) ZSOCK_IN6_IS_ADDR_LINKLOCAL((const struct zsock_in6_addr *)(a))

/** @brief Test for a unicast site-local IPv6 address. */
#define IN6_IS_ADDR_SITELOCAL(a) ZSOCK_IN6_IS_ADDR_SITELOCAL((const struct zsock_in6_addr *)(a))

/** @brief Test for an IPv4-mapped IPv6 address. */
#define IN6_IS_ADDR_V4MAPPED(a) ZSOCK_IN6_IS_ADDR_V4MAPPED((const struct zsock_in6_addr *)(a))

/** @brief Test for an IPv4-compatible IPv6 address. */
#define IN6_IS_ADDR_V4COMPAT(a) ZSOCK_IN6_IS_ADDR_V4COMPAT((const struct zsock_in6_addr *)(a))

/** @brief Test for a multicast node-local IPv6 address. */
#define IN6_IS_ADDR_MC_NODELOCAL(a)                                                                \
	ZSOCK_IN6_IS_ADDR_MC_NODELOCAL((const struct zsock_in6_addr *)(a))

/** @brief Test for a multicast link-local IPv6 address. */
#define IN6_IS_ADDR_MC_LINKLOCAL(a)                                                                \
	ZSOCK_IN6_IS_ADDR_MC_LINKLOCAL((const struct zsock_in6_addr *)(a))

/** @brief Test for a multicast site-local IPv6 address. */
#define IN6_IS_ADDR_MC_SITELOCAL(a)                                                                \
	ZSOCK_IN6_IS_ADDR_MC_SITELOCAL((const struct zsock_in6_addr *)(a))

/** @brief Test for a multicast organization-local IPv6 address. */
#define IN6_IS_ADDR_MC_ORGLOCAL(a) ZSOCK_IN6_IS_ADDR_MC_ORGLOCAL((const struct zsock_in6_addr *)(a))

/** @brief Test for a multicast global IPv6 address. */
#define IN6_IS_ADDR_MC_GLOBAL(a) ZSOCK_IN6_IS_ADDR_MC_GLOBAL((const struct zsock_in6_addr *)(a))

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_ */

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

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_SA_FAMILY_T_DECLARED) && !defined(__sa_family_t_defined)
typedef net_sa_family_t sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
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

#if !(defined(_IN_ADDR_DECLARED) || defined(__in_addr_defined)) || defined(__DOXYGEN__)
/** @brief IPv4 address. */
struct in_addr {
	in_addr_t s_addr; /**< IPv4 address in network byte order. */
};
#define _IN_ADDR_DECLARED
#define __in_addr_defined
#endif

#if !(defined(_SOCKADDR_IN_DECLARED) || defined(__sockaddr_in_defined)) || defined(__DOXYGEN__)
/** @brief Socket address for IPv4. */
struct sockaddr_in {
	sa_family_t    sin_family; /**< Address family; @ref AF_INET. */
	in_port_t      sin_port;   /**< Port number in network byte order. */
	struct in_addr sin_addr;   /**< IPv4 address. */
};
#define _SOCKADDR_IN_DECLARED
#define __sockaddr_in_defined
#endif

#if !(defined(_IN6_ADDR_DECLARED) || defined(__in6_addr_defined)) || defined(__DOXYGEN__)
/** @brief IPv6 address. */
struct in6_addr {
	uint8_t  s6_addr[16]; /**< IPv6 address bytes in network byte order. */
};
#define _IN6_ADDR_DECLARED
#define __in6_addr_defined
#endif

#if !(defined(_SOCKADDR_IN6_DECLARED) || defined(__sockaddr_in6_defined)) || defined(__DOXYGEN__)
/** @brief Socket address for IPv6. */
struct sockaddr_in6 {
	sa_family_t     sin6_family;   /**< Address family; @ref AF_INET6. */
	in_port_t       sin6_port;     /**< Port number in network byte order. */
	struct in6_addr sin6_addr;     /**< IPv6 address. */
	uint32_t        sin6_scope_id; /**< Scope identifier. */
	/* slightly out of order w.r.t. spec (zephyr assumes consistent family, port, addr offsets) */
	uint32_t        sin6_flowinfo; /**< IPv6 flow information. */
};
#define _SOCKADDR_IN6_DECLARED
#define __sockaddr_in6_defined
#endif

/** @brief IPv6 wildcard address. */
extern const struct in6_addr in6addr_any;
/** @brief IPv6 loopback address. */
extern const struct in6_addr in6addr_loopback;

/** @brief IPv6 wildcard address initializer. */
#define IN6ADDR_ANY_INIT      { { 0 } }
/** @brief IPv6 loopback address initializer. */
#define IN6ADDR_LOOPBACK_INIT { { 0, 0, 0, 0, 0, 0, 0, 0, \
				  0, 0, 0, 0, 0, 0, 0, 1 } }

#if !(defined(_IPV6_MREQ_DECLARED) || defined(__ipv6_mreq_defined)) || defined(__DOXYGEN__)
/** @brief IPv6 multicast group request. */
struct ipv6_mreq {
	struct in6_addr ipv6mr_multiaddr; /**< Multicast address. */
	unsigned int    ipv6mr_interface; /**< Interface index. */
};
#define _IPV6_MREQ_DECLARED
#define __ipv6_mreq_defined
#endif

/* Protocols (IPPROTO_*). */
#define IPPROTO_IP   NET_IPPROTO_IP   /**< Dummy protocol for IP. */
#define IPPROTO_IPV6 NET_IPPROTO_IPV6 /**< IPv6 header. */
#define IPPROTO_ICMP NET_IPPROTO_ICMP /**< Internet Control Message Protocol. */
#define IPPROTO_RAW  NET_IPPROTO_RAW  /**< Raw IP packets. */
#define IPPROTO_TCP  NET_IPPROTO_TCP  /**< Transmission Control Protocol. */
#define IPPROTO_UDP  NET_IPPROTO_UDP  /**< User Datagram Protocol. */

/** @brief IPv4 wildcard address (0.0.0.0). */
#define INADDR_ANY       ((in_addr_t)NET_INADDR_ANY)
/** @brief IPv4 broadcast address. */
#define INADDR_BROADCAST ((in_addr_t)NET_INADDR_BROADCAST)

/** @brief Maximum length of an IPv4 address string. */
#define INET_ADDRSTRLEN NET_INET_ADDRSTRLEN

/** @brief Convert a 32-bit value from host to network byte order. */
#define htonl(x) net_htonl(x)
/** @brief Convert a 16-bit value from host to network byte order. */
#define htons(x) net_htons(x)
/** @brief Convert a 32-bit value from network to host byte order. */
#define ntohl(x) net_ntohl(x)
/** @brief Convert a 16-bit value from network to host byte order. */
#define ntohs(x) net_ntohs(x)

/** @brief Maximum length of an IPv6 address string. */
#define INET6_ADDRSTRLEN NET_INET6_ADDRSTRLEN

/* IPv6-level socket options (IPPROTO_IPV6). */
/** @brief Join an IPv6 multicast group. */
#define IPV6_JOIN_GROUP ZSOCK_IPV6_JOIN_GROUP
/** @brief Leave an IPv6 multicast group. */
#define IPV6_LEAVE_GROUP ZSOCK_IPV6_LEAVE_GROUP
/** @brief Hop limit for multicast packets. */
#define IPV6_MULTICAST_HOPS ZSOCK_IPV6_MULTICAST_HOPS
/** @brief Outgoing interface for multicast packets. */
#define IPV6_MULTICAST_IF ZSOCK_IPV6_MULTICAST_IF
/** @brief Loopback of outgoing multicast packets. */
#define IPV6_MULTICAST_LOOP ZSOCK_IPV6_MULTICAST_LOOP
/** @brief Hop limit for unicast packets. */
#define IPV6_UNICAST_HOPS ZSOCK_IPV6_UNICAST_HOPS
/** @brief Restrict the socket to IPv6 communication only. */
#define IPV6_V6ONLY ZSOCK_IPV6_V6ONLY

/* IPv6 address tests. Each takes a pointer to a struct in6_addr and evaluates non-zero on match. */
/** @brief Test whether an IPv6 address is the unspecified address (`::`). */
#define IN6_IS_ADDR_UNSPECIFIED(addr) ZSOCK_IN6_IS_ADDR_UNSPECIFIED(addr)
/** @brief Test whether an IPv6 address is the loopback address (`::1`). */
#define IN6_IS_ADDR_LOOPBACK(addr) ZSOCK_IN6_IS_ADDR_LOOPBACK(addr)
/** @brief Test whether an IPv6 address is a multicast address (`ff00::/8`). */
#define IN6_IS_ADDR_MULTICAST(addr) ZSOCK_IN6_IS_ADDR_MULTICAST(addr)
/** @brief Test whether an IPv6 address is link-local unicast (`fe80::/10`). */
#define IN6_IS_ADDR_LINKLOCAL(addr) ZSOCK_IN6_IS_ADDR_LINKLOCAL(addr)
/** @brief Test whether an IPv6 address is site-local unicast (`fec0::/10`). */
#define IN6_IS_ADDR_SITELOCAL(addr) ZSOCK_IN6_IS_ADDR_SITELOCAL(addr)
/** @brief Test whether an IPv6 address is an IPv4-mapped IPv6 address. */
#define IN6_IS_ADDR_V4MAPPED(addr) ZSOCK_IN6_IS_ADDR_V4MAPPED(addr)
/** @brief Test whether an IPv6 multicast address has node-local scope (`ff01::/8`). */
#define IN6_IS_ADDR_MC_NODELOCAL(addr) ZSOCK_IN6_IS_ADDR_MC_NODELOCAL(addr)
/** @brief Test whether an IPv6 multicast address has link-local scope (`ff02::/8`). */
#define IN6_IS_ADDR_MC_LINKLOCAL(addr) ZSOCK_IN6_IS_ADDR_MC_LINKLOCAL(addr)
/** @brief Test whether an IPv6 multicast address has site-local scope (`ff03::/8`). */
#define IN6_IS_ADDR_MC_SITELOCAL(addr) ZSOCK_IN6_IS_ADDR_MC_SITELOCAL(addr)
/** @brief Test whether an IPv6 multicast address has organization-local scope (`ff04::/8`). */
#define IN6_IS_ADDR_MC_ORGLOCAL(addr) ZSOCK_IN6_IS_ADDR_MC_ORGLOCAL(addr)
/** @brief Test whether an IPv6 multicast address has global scope (`ff00::/8`). */
#define IN6_IS_ADDR_MC_GLOBAL(addr) ZSOCK_IN6_IS_ADDR_MC_GLOBAL(addr)

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_IN_H_ */

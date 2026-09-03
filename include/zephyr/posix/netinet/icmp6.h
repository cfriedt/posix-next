/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ICMPv6 definitions (<netinet/icmp6.h>)
 *
 * BSD/Linux-compatible ICMPv6 message layout, types, codes, and the ICMPv6 type
 * filter. This header is a de-facto (Linux/BSD) extension, not part of POSIX.1; it
 * is provided for source compatibility. The message types and codes reuse the
 * Zephyr values from <zephyr/net/net_ip.h>; every identifier is guarded so an
 * application may define its own before including this header.
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETINET_ICMP6_H_
#define ZEPHYR_INCLUDE_POSIX_NETINET_ICMP6_H_

#include <stdint.h>
#include <string.h>

#include <netinet/in.h>
#include <zephyr/net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief ICMPv6 header (RFC 4443). */
struct icmp6_hdr {
	uint8_t icmp6_type;   /**< Message type. */
	uint8_t icmp6_code;   /**< Type sub-code. */
	uint16_t icmp6_cksum; /**< Checksum. */
	union {
		uint32_t icmp6_un_data32[1]; /**< Type-specific field. */
		uint16_t icmp6_un_data16[2]; /**< Type-specific field. */
		uint8_t icmp6_un_data8[4];   /**< Type-specific field. */
	} icmp6_dataun;
};

#define icmp6_data32   icmp6_dataun.icmp6_un_data32
#define icmp6_data16   icmp6_dataun.icmp6_un_data16
#define icmp6_data8    icmp6_dataun.icmp6_un_data8
#define icmp6_pptr     icmp6_data32[0] /**< Parameter problem pointer. */
#define icmp6_mtu      icmp6_data32[0] /**< Packet too big MTU. */
#define icmp6_id       icmp6_data16[0] /**< Echo identifier. */
#define icmp6_seq      icmp6_data16[1] /**< Echo sequence number. */
#define icmp6_maxdelay icmp6_data16[0] /**< MLD maximum response delay. */

/* Message types. */
#ifndef ICMP6_DST_UNREACH
#define ICMP6_DST_UNREACH NET_ICMPV6_DST_UNREACH /**< Destination unreachable. */
#endif
#ifndef ICMP6_PACKET_TOO_BIG
#define ICMP6_PACKET_TOO_BIG NET_ICMPV6_PACKET_TOO_BIG /**< Packet too big. */
#endif
#ifndef ICMP6_TIME_EXCEEDED
#define ICMP6_TIME_EXCEEDED NET_ICMPV6_TIME_EXCEEDED /**< Time exceeded. */
#endif
#ifndef ICMP6_PARAM_PROB
#define ICMP6_PARAM_PROB NET_ICMPV6_PARAM_PROBLEM /**< Parameter problem. */
#endif
#ifndef ICMP6_ECHO_REQUEST
#define ICMP6_ECHO_REQUEST NET_ICMPV6_ECHO_REQUEST /**< Echo request. */
#endif
#ifndef ICMP6_ECHO_REPLY
#define ICMP6_ECHO_REPLY NET_ICMPV6_ECHO_REPLY /**< Echo reply. */
#endif
#ifndef MLD_LISTENER_QUERY
#define MLD_LISTENER_QUERY NET_ICMPV6_MLD_QUERY /**< Multicast listener query. */
#endif
#ifndef MLD_LISTENER_REPORT
#define MLD_LISTENER_REPORT NET_ICMPV6_MLD_REPORT /**< Multicast listener report. */
#endif
#ifndef MLD_LISTENER_REDUCTION
#define MLD_LISTENER_REDUCTION NET_ICMPV6_MLD_DONE /**< Multicast listener done. */
#endif
#ifndef ICMP6_MEMBERSHIP_QUERY
#define ICMP6_MEMBERSHIP_QUERY MLD_LISTENER_QUERY
#endif
#ifndef ICMP6_MEMBERSHIP_REPORT
#define ICMP6_MEMBERSHIP_REPORT MLD_LISTENER_REPORT
#endif
#ifndef ICMP6_MEMBERSHIP_REDUCTION
#define ICMP6_MEMBERSHIP_REDUCTION MLD_LISTENER_REDUCTION
#endif
#ifndef ND_ROUTER_SOLICIT
#define ND_ROUTER_SOLICIT NET_ICMPV6_RS /**< Router solicitation. */
#endif
#ifndef ND_ROUTER_ADVERT
#define ND_ROUTER_ADVERT NET_ICMPV6_RA /**< Router advertisement. */
#endif
#ifndef ND_NEIGHBOR_SOLICIT
#define ND_NEIGHBOR_SOLICIT NET_ICMPV6_NS /**< Neighbor solicitation. */
#endif
#ifndef ND_NEIGHBOR_ADVERT
#define ND_NEIGHBOR_ADVERT NET_ICMPV6_NA /**< Neighbor advertisement. */
#endif
#ifndef ND_REDIRECT
#define ND_REDIRECT NET_ICMPV6_REDIRECT /**< Redirect. */
#endif
#ifndef ICMP6_INFOMSG_MASK
#define ICMP6_INFOMSG_MASK 0x80 /**< Set in all informational message types. */
#endif

/* Destination unreachable codes. */
#ifndef ICMP6_DST_UNREACH_NOROUTE
#define ICMP6_DST_UNREACH_NOROUTE NET_ICMPV6_DST_UNREACH_NO_ROUTE
#endif
#ifndef ICMP6_DST_UNREACH_ADMIN
#define ICMP6_DST_UNREACH_ADMIN NET_ICMPV6_DST_UNREACH_ADMIN
#endif
#ifndef ICMP6_DST_UNREACH_BEYONDSCOPE
#define ICMP6_DST_UNREACH_BEYONDSCOPE NET_ICMPV6_DST_UNREACH_SCOPE
#endif
#ifndef ICMP6_DST_UNREACH_ADDR
#define ICMP6_DST_UNREACH_ADDR NET_ICMPV6_DST_UNREACH_NO_ADDR
#endif
#ifndef ICMP6_DST_UNREACH_NOPORT
#define ICMP6_DST_UNREACH_NOPORT NET_ICMPV6_DST_UNREACH_NO_PORT
#endif

/* Time exceeded codes. */
#ifndef ICMP6_TIME_EXCEED_TRANSIT
#define ICMP6_TIME_EXCEED_TRANSIT 0
#endif
#ifndef ICMP6_TIME_EXCEED_REASSEMBLY
#define ICMP6_TIME_EXCEED_REASSEMBLY 1
#endif

/* Parameter problem codes. */
#ifndef ICMP6_PARAMPROB_HEADER
#define ICMP6_PARAMPROB_HEADER NET_ICMPV6_PARAM_PROB_HEADER
#endif
#ifndef ICMP6_PARAMPROB_NEXTHEADER
#define ICMP6_PARAMPROB_NEXTHEADER NET_ICMPV6_PARAM_PROB_NEXTHEADER
#endif
#ifndef ICMP6_PARAMPROB_OPTION
#define ICMP6_PARAMPROB_OPTION NET_ICMPV6_PARAM_PROB_OPTION
#endif

/** @brief ICMPv6 message type filter (RFC 3542). */
struct icmp6_filter {
	uint32_t icmp6_filt[8]; /**< One bit per message type. */
};

#define ICMP6_FILTER_WILLPASS(type, filterp)                                                       \
	((((filterp)->icmp6_filt[(type) >> 5]) & (1U << ((type) & 31))) == 0)
#define ICMP6_FILTER_WILLBLOCK(type, filterp)                                                      \
	((((filterp)->icmp6_filt[(type) >> 5]) & (1U << ((type) & 31))) != 0)
#define ICMP6_FILTER_SETPASS(type, filterp)                                                        \
	((((filterp)->icmp6_filt[(type) >> 5]) &= ~(1U << ((type) & 31))))
#define ICMP6_FILTER_SETBLOCK(type, filterp)                                                       \
	((((filterp)->icmp6_filt[(type) >> 5]) |= (1U << ((type) & 31))))
#define ICMP6_FILTER_SETPASSALL(filterp) memset((filterp), 0, sizeof(struct icmp6_filter))
#define ICMP6_FILTER_SETBLOCKALL(filterp) memset((filterp), 0xff, sizeof(struct icmp6_filter))

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_ICMP6_H_ */

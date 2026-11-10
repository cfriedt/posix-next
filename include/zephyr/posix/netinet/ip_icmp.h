/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ICMPv4 definitions (<netinet/ip_icmp.h>)
 *
 * BSD/Linux-compatible ICMPv4 message layouts, types, and codes. This header is
 * a de-facto (Linux/BSD) extension, not part of POSIX.1; it is provided for source
 * compatibility. The message types and codes reuse the Zephyr values from
 * <zephyr/net/net_ip.h>; every identifier is guarded so an application may define
 * its own before including this header.
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETINET_IP_ICMP_H_
#define ZEPHYR_INCLUDE_POSIX_NETINET_IP_ICMP_H_

#include <stdint.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <zephyr/net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief ICMPv4 header, Linux layout. */
struct icmphdr {
	uint8_t type;      /**< Message type. */
	uint8_t code;      /**< Type sub-code. */
	uint16_t checksum; /**< Checksum. */
	union {
		struct {
			uint16_t id;       /**< Echo identifier. */
			uint16_t sequence; /**< Echo sequence number. */
		} echo;
		uint32_t gateway; /**< Redirect gateway address. */
		struct {
			uint16_t reserved;
			uint16_t mtu; /**< Path MTU. */
		} frag;
	} un;
};

/** @brief ICMPv4 message, BSD layout (RFC 792). */
struct icmp {
	uint8_t icmp_type;   /**< Message type. */
	uint8_t icmp_code;   /**< Type sub-code. */
	uint16_t icmp_cksum; /**< Checksum. */
	union {
		uint8_t ih_pptr;          /**< Parameter problem pointer. */
		struct in_addr ih_gwaddr; /**< Redirect gateway address. */
		struct ih_idseq {
			uint16_t icd_id;  /**< Echo identifier. */
			uint16_t icd_seq; /**< Echo sequence number. */
		} ih_idseq;
		uint32_t ih_void;
		struct ih_pmtu {
			uint16_t ipm_void;
			uint16_t ipm_nextmtu; /**< Path MTU. */
		} ih_pmtu;
		struct ih_rtradv {
			uint8_t irt_num_addrs;
			uint8_t irt_wpa;
			uint16_t irt_lifetime;
		} ih_rtradv;
	} icmp_hun;
	union {
		struct {
			uint32_t its_otime; /**< Originate timestamp. */
			uint32_t its_rtime; /**< Receive timestamp. */
			uint32_t its_ttime; /**< Transmit timestamp. */
		} id_ts;
		struct {
			struct ip idi_ip; /**< Offending IP header. */
			/* options and then 64 bits of data */
		} id_ip;
		uint32_t id_mask;  /**< Address mask. */
		uint8_t id_data[1]; /**< Message data. */
	} icmp_dun;
};

#define icmp_pptr     icmp_hun.ih_pptr
#define icmp_gwaddr   icmp_hun.ih_gwaddr
#define icmp_id       icmp_hun.ih_idseq.icd_id
#define icmp_seq      icmp_hun.ih_idseq.icd_seq
#define icmp_void     icmp_hun.ih_void
#define icmp_pmvoid   icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu  icmp_hun.ih_pmtu.ipm_nextmtu
#define icmp_otime    icmp_dun.id_ts.its_otime
#define icmp_rtime    icmp_dun.id_ts.its_rtime
#define icmp_ttime    icmp_dun.id_ts.its_ttime
#define icmp_ip       icmp_dun.id_ip.idi_ip
#define icmp_mask     icmp_dun.id_mask
#define icmp_data     icmp_dun.id_data

#ifndef ICMP_MINLEN
#define ICMP_MINLEN 8 /**< Minimum ICMP message length: type, code, checksum, and 4 bytes. */
#endif

/* Message types: Linux names. */
#ifndef ICMP_ECHOREPLY
#define ICMP_ECHOREPLY NET_ICMPV4_ECHO_REPLY /**< Echo reply. */
#endif
#ifndef ICMP_DEST_UNREACH
#define ICMP_DEST_UNREACH NET_ICMPV4_DST_UNREACH /**< Destination unreachable. */
#endif
#ifndef ICMP_SOURCE_QUENCH
#define ICMP_SOURCE_QUENCH NET_ICMPV4_SOURCE_QUENCH /**< Source quench. */
#endif
#ifndef ICMP_REDIRECT
#define ICMP_REDIRECT NET_ICMPV4_REDIRECT /**< Redirect. */
#endif
#ifndef ICMP_ECHO
#define ICMP_ECHO NET_ICMPV4_ECHO_REQUEST /**< Echo request. */
#endif
#ifndef ICMP_TIME_EXCEEDED
#define ICMP_TIME_EXCEEDED NET_ICMPV4_TIME_EXCEEDED /**< Time exceeded. */
#endif
#ifndef ICMP_PARAMETERPROB
#define ICMP_PARAMETERPROB NET_ICMPV4_BAD_IP_HEADER /**< Parameter problem. */
#endif
#ifndef ICMP_TIMESTAMP
#define ICMP_TIMESTAMP NET_ICMPV4_TIMESTAMP /**< Timestamp request. */
#endif
#ifndef ICMP_TIMESTAMPREPLY
#define ICMP_TIMESTAMPREPLY NET_ICMPV4_TIMESTAMP_REPLY /**< Timestamp reply. */
#endif
#ifndef ICMP_INFO_REQUEST
#define ICMP_INFO_REQUEST NET_ICMPV4_INFO_REQUEST /**< Information request. */
#endif
#ifndef ICMP_INFO_REPLY
#define ICMP_INFO_REPLY NET_ICMPV4_INFO_REPLY /**< Information reply. */
#endif
#ifndef ICMP_ADDRESS
#define ICMP_ADDRESS NET_ICMPV4_ADDR_MASK /**< Address mask request. */
#endif
#ifndef ICMP_ADDRESSREPLY
#define ICMP_ADDRESSREPLY NET_ICMPV4_ADDR_MASK_REPLY /**< Address mask reply. */
#endif

/* Message types: BSD names. */
#ifndef ICMP_UNREACH
#define ICMP_UNREACH ICMP_DEST_UNREACH
#endif
#ifndef ICMP_SOURCEQUENCH
#define ICMP_SOURCEQUENCH ICMP_SOURCE_QUENCH
#endif
#ifndef ICMP_ROUTERADVERT
#define ICMP_ROUTERADVERT NET_ICMPV4_ROUTER_ADVERT
#endif
#ifndef ICMP_ROUTERSOLICIT
#define ICMP_ROUTERSOLICIT NET_ICMPV4_ROUTER_SOLICIT
#endif
#ifndef ICMP_TIMXCEED
#define ICMP_TIMXCEED ICMP_TIME_EXCEEDED
#endif
#ifndef ICMP_PARAMPROB
#define ICMP_PARAMPROB ICMP_PARAMETERPROB
#endif
#ifndef ICMP_TSTAMP
#define ICMP_TSTAMP ICMP_TIMESTAMP
#endif
#ifndef ICMP_TSTAMPREPLY
#define ICMP_TSTAMPREPLY ICMP_TIMESTAMPREPLY
#endif
#ifndef ICMP_IREQ
#define ICMP_IREQ ICMP_INFO_REQUEST
#endif
#ifndef ICMP_IREQREPLY
#define ICMP_IREQREPLY ICMP_INFO_REPLY
#endif
#ifndef ICMP_MASKREQ
#define ICMP_MASKREQ ICMP_ADDRESS
#endif
#ifndef ICMP_MASKREPLY
#define ICMP_MASKREPLY ICMP_ADDRESSREPLY
#endif

/* Destination unreachable codes. */
#ifndef ICMP_NET_UNREACH
#define ICMP_NET_UNREACH NET_ICMPV4_DST_UNREACH_NO_NET /**< Network unreachable. */
#endif
#ifndef ICMP_HOST_UNREACH
#define ICMP_HOST_UNREACH NET_ICMPV4_DST_UNREACH_NO_HOST /**< Host unreachable. */
#endif
#ifndef ICMP_PROT_UNREACH
#define ICMP_PROT_UNREACH NET_ICMPV4_DST_UNREACH_NO_PROTO /**< Protocol unreachable. */
#endif
#ifndef ICMP_PORT_UNREACH
#define ICMP_PORT_UNREACH NET_ICMPV4_DST_UNREACH_NO_PORT /**< Port unreachable. */
#endif
#ifndef ICMP_FRAG_NEEDED
#define ICMP_FRAG_NEEDED NET_ICMPV4_DST_UNREACH_FRAG /**< Fragmentation needed. */
#endif
#ifndef ICMP_SR_FAILED
#define ICMP_SR_FAILED NET_ICMPV4_DST_UNREACH_SRC_ROUTE /**< Source route failed. */
#endif

/* Time exceeded codes. */
#ifndef ICMP_EXC_TTL
#define ICMP_EXC_TTL 0 /**< TTL count exceeded. */
#endif
#ifndef ICMP_EXC_FRAGTIME
#define ICMP_EXC_FRAGTIME 1 /**< Fragment reassembly time exceeded. */
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_IP_ICMP_H_ */

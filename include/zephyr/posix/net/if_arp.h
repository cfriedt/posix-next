/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ARP definitions (<net/if_arp.h>)
 *
 * Linux/BSD-compatible ARP hardware identifiers, protocol opcodes, and packet
 * header layout. This header is a de-facto (Linux/BSD) extension, not part of
 * POSIX.1; it is provided for source compatibility. ARP hardware types that
 * Zephyr already defines reuse the Zephyr value (@c NET_ARPHRD_ETHER,
 * @c NET_ARPHRD_PPP); every identifier is guarded so an application may
 * define its own before including this header.
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NET_IF_ARP_H_
#define ZEPHYR_INCLUDE_POSIX_NET_IF_ARP_H_

#include <stdint.h>

#include <zephyr/net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ARP protocol opcodes. */
#ifndef ARPOP_REQUEST
#define ARPOP_REQUEST 1 /**< ARP request. */
#endif
#ifndef ARPOP_REPLY
#define ARPOP_REPLY 2 /**< ARP reply. */
#endif
#ifndef ARPOP_RREQUEST
#define ARPOP_RREQUEST 3 /**< RARP request. */
#endif
#ifndef ARPOP_RREPLY
#define ARPOP_RREPLY 4 /**< RARP reply. */
#endif
#ifndef ARPOP_InREQUEST
#define ARPOP_InREQUEST 8 /**< InARP request. */
#endif
#ifndef ARPOP_InREPLY
#define ARPOP_InREPLY 9 /**< InARP reply. */
#endif
#ifndef ARPOP_NAK
#define ARPOP_NAK 10 /**< (ATM) ARP NAK. */
#endif

/* ARP protocol hardware identifiers. */
#ifndef ARPHRD_ETHER
#define ARPHRD_ETHER NET_ARPHRD_ETHER /**< Ethernet 10/100Mbps. */
#endif
#ifndef ARPHRD_IEEE802
#define ARPHRD_IEEE802 6 /**< IEEE 802.2 Ethernet/TR/TB. */
#endif
#ifndef ARPHRD_ARCNET
#define ARPHRD_ARCNET 7 /**< ARCnet. */
#endif
#ifndef ARPHRD_IEEE1394
#define ARPHRD_IEEE1394 24 /**< IEEE 1394 IPv4 - RFC 2734. */
#endif
#ifndef ARPHRD_EUI64
#define ARPHRD_EUI64 27 /**< EUI-64. */
#endif
#ifndef ARPHRD_INFINIBAND
#define ARPHRD_INFINIBAND 32 /**< InfiniBand. */
#endif
#ifndef ARPHRD_CAN
#define ARPHRD_CAN 280 /**< Controller Area Network. */
#endif
#ifndef ARPHRD_IEEE802154
#define ARPHRD_IEEE802154 804 /**< IEEE 802.15.4. */
#endif
#ifndef ARPHRD_PPP
#define ARPHRD_PPP NET_ARPHRD_PPP /**< Point-to-Point Protocol. */
#endif
#ifndef ARPHRD_LOOPBACK
#define ARPHRD_LOOPBACK 772 /**< Loopback device. */
#endif

/** @brief ARP packet header (RFC 826). */
struct arphdr {
	uint16_t ar_hrd; /**< Format of hardware address. */
	uint16_t ar_pro; /**< Format of protocol address. */
	uint8_t ar_hln;  /**< Length of hardware address. */
	uint8_t ar_pln;  /**< Length of protocol address. */
	uint16_t ar_op;  /**< ARP opcode (command). */
};

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NET_IF_ARP_H_ */

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Packet socket address (<netpacket/packet.h>)
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETPACKET_PACKET_H_
#define ZEPHYR_INCLUDE_POSIX_NETPACKET_PACKET_H_

#include <stdint.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_SOCKADDR_LL_DECLARED) || defined(__sockaddr_ll_defined)) || defined(__DOXYGEN__)
/** @brief Link-layer socket address. */
struct sockaddr_ll {
	/** @brief Always @c AF_PACKET. */
	sa_family_t sll_family;
	/** @brief Physical-layer protocol. */
	uint16_t sll_protocol;
	/** @brief Interface number. */
	int sll_ifindex;
	/** @brief ARP hardware type. */
	uint16_t sll_hatype;
	/** @brief Packet type. */
	uint8_t sll_pkttype;
	/** @brief Length of @p sll_addr. */
	uint8_t sll_halen;
	/** @brief Physical-layer address. */
	uint8_t sll_addr[8];
};
#define _SOCKADDR_LL_DECLARED
#define __sockaddr_ll_defined
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETPACKET_PACKET_H_ */

/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief IPv4 header definitions (<netinet/ip.h>)
 *
 * BSD/Linux-compatible IPv4 header layouts and field constants. This header is a
 * de-facto (Linux/BSD) extension, not part of POSIX.1; it is provided for source
 * compatibility. Values that Zephyr already defines reuse the Zephyr definition;
 * every identifier is guarded so an application may define its own before
 * including this header.
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETINET_IP_H_
#define ZEPHYR_INCLUDE_POSIX_NETINET_IP_H_

#include <stdint.h>

#include <netinet/in.h>
#include <zephyr/net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief IPv4 header, BSD layout (RFC 791). */
struct ip {
#ifndef CONFIG_BIG_ENDIAN
	unsigned int ip_hl: 4; /**< Header length (32-bit words). */
	unsigned int ip_v: 4;  /**< Version. */
#else
	unsigned int ip_v: 4;  /**< Version. */
	unsigned int ip_hl: 4; /**< Header length (32-bit words). */
#endif
	uint8_t ip_tos;        /**< Type of service. */
	uint16_t ip_len;       /**< Total length. */
	uint16_t ip_id;        /**< Identification. */
	uint16_t ip_off;       /**< Fragment offset field. */
	uint8_t ip_ttl;        /**< Time to live. */
	uint8_t ip_p;          /**< Protocol. */
	uint16_t ip_sum;       /**< Checksum. */
	struct in_addr ip_src; /**< Source address. */
	struct in_addr ip_dst; /**< Destination address. */
};

/** @brief IPv4 header, Linux layout. */
struct iphdr {
#ifndef CONFIG_BIG_ENDIAN
	unsigned int ihl: 4;     /**< Header length (32-bit words). */
	unsigned int version: 4; /**< Version. */
#else
	unsigned int version: 4; /**< Version. */
	unsigned int ihl: 4;     /**< Header length (32-bit words). */
#endif
	uint8_t tos;      /**< Type of service. */
	uint16_t tot_len; /**< Total length. */
	uint16_t id;      /**< Identification. */
	uint16_t frag_off; /**< Fragment offset field. */
	uint8_t ttl;      /**< Time to live. */
	uint8_t protocol; /**< Protocol. */
	uint16_t check;   /**< Checksum. */
	uint32_t saddr;   /**< Source address. */
	uint32_t daddr;   /**< Destination address. */
};

#ifndef IPVERSION
#define IPVERSION 4 /**< IP version number. */
#endif
#ifndef IP_MAXPACKET
#define IP_MAXPACKET 65535 /**< Maximum packet size. */
#endif
#ifndef MAXTTL
#define MAXTTL 255 /**< Maximum time to live. */
#endif
#ifndef IPDEFTTL
#define IPDEFTTL 64 /**< Default time to live. */
#endif

/* Fragment offset field bits. */
#ifndef IP_RF
#define IP_RF 0x8000 /**< Reserved fragment flag. */
#endif
#ifndef IP_DF
#define IP_DF 0x4000 /**< Don't fragment flag. */
#endif
#ifndef IP_MF
#define IP_MF NET_IPV4_MORE_FRAG_MASK /**< More fragments flag. */
#endif
#ifndef IP_OFFMASK
#define IP_OFFMASK NET_IPV4_FRAGH_OFFSET_MASK /**< Mask for the fragmenting bits. */
#endif

/* Type of service bits. */
#ifndef IPTOS_LOWDELAY
#define IPTOS_LOWDELAY 0x10 /**< Minimize delay. */
#endif
#ifndef IPTOS_THROUGHPUT
#define IPTOS_THROUGHPUT 0x08 /**< Maximize throughput. */
#endif
#ifndef IPTOS_RELIABILITY
#define IPTOS_RELIABILITY 0x04 /**< Maximize reliability. */
#endif
#ifndef IPTOS_MINCOST
#define IPTOS_MINCOST 0x02 /**< Minimize monetary cost. */
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETINET_IP_H_ */

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Network interface addresses (<ifaddrs.h>)
 *
 * Not part of POSIX: a BSD extension, provided for compatibility with the
 * many applications that enumerate interfaces through it.
 *
 * @see <a href="https://man7.org/linux/man-pages/man3/getifaddrs.3.html">getifaddrs(3)</a>
 */

#ifndef ZEPHYR_INCLUDE_POSIX_IFADDRS_H_
#define ZEPHYR_INCLUDE_POSIX_IFADDRS_H_

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief One interface address: an element of the getifaddrs() list. */
struct ifaddrs {
	struct ifaddrs *ifa_next;     /**< Next element, or NULL. */
	char *ifa_name;               /**< Interface name. */
	unsigned int ifa_flags;       /**< IFF_* flags, as from SIOCGIFFLAGS. */
	struct sockaddr *ifa_addr;    /**< Interface address, or NULL. */
	struct sockaddr *ifa_netmask; /**< Netmask, or NULL. */
	struct sockaddr *ifa_dstaddr; /**< Broadcast or peer address, or NULL. */
	void *ifa_data;               /**< Address-family specific data, or NULL. */
};

/** @brief The broadcast-address reading of ifa_dstaddr. */
#define ifa_broadaddr ifa_dstaddr

/**
 * @brief Enumerate every address of every network interface.
 *
 * @param ifap Output: head of an allocated list, released with freeifaddrs().
 * @return 0 on success, or -1 with errno set on failure.
 */
int getifaddrs(struct ifaddrs **ifap);

/**
 * @brief Release a list returned by getifaddrs().
 *
 * @param ifa Head of the list, or NULL.
 */
void freeifaddrs(struct ifaddrs *ifa);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_IFADDRS_H_ */

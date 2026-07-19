/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Network interface names and indices (<net/if.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/net_if.h.html">
 *      POSIX.1-2017 &lt;net/if.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NET_IF_H_
#define ZEPHYR_INCLUDE_POSIX_NET_IF_H_

#include <zephyr/net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum length of a network interface name including the NUL terminator. */
#define IF_NAMESIZE NET_IFNAMSIZ

#if !(defined(_IF_NAMEINDEX_DECLARED) || defined(__if_nameindex_defined)) || defined(__DOXYGEN__)
/** @brief Network interface name-to-index mapping. */
struct if_nameindex {
	unsigned int if_index; /**< Numeric interface index. */
	char *if_name;         /**< Interface name string. */
};
#define _IF_NAMEINDEX_DECLARED
#define __if_nameindex_defined
#endif

/**
 * @brief Map a network interface index to its name.
 * @param ifindex Numeric interface index.
 * @param ifname  Output buffer of at least @ref IF_NAMESIZE bytes.
 * @return @p ifname on success, or @c NULL with errno set on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/if_indextoname.html
 */
char *if_indextoname(unsigned int ifindex, char *ifname);

/**
 * @brief Free a list returned by if_nameindex().
 * @param ptr List to free (must have been returned by if_nameindex()).
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/if_freenameindex.html
 */
void if_freenameindex(struct if_nameindex *ptr);

/**
 * @brief Return all network interfaces as a name-index array.
 *
 * The returned array is terminated by an entry with @c if_index == 0 and
 * @c if_name == @c NULL.  Free with @ref if_freenameindex().
 *
 * @return Allocated array on success, or @c NULL with errno set on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/if_nameindex.html
 */
struct if_nameindex *if_nameindex(void);

/**
 * @brief Map a network interface name to its index.
 * @param ifname Interface name string.
 * @return Interface index on success, or 0 with errno set on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/if_nametoindex.html
 */
unsigned int if_nametoindex(const char *ifname);


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NET_IF_H_ */

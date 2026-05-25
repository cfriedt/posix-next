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

#ifdef CONFIG_NET_INTERFACE_NAME_LEN
/** @brief Maximum length of a network interface name including the NUL terminator.  @ingroup posix_option_group_networking*/
#define IF_NAMESIZE CONFIG_NET_INTERFACE_NAME_LEN
#else
#define IF_NAMESIZE 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Network interface name-to-index mapping. */
struct if_nameindex {
	unsigned int if_index; /**< Numeric interface index. */
	char *if_name;         /**< Interface name string. */
};

/**
 * @brief Map a network interface index to its name.
 * @ingroup posix_option_group_networking
 * @param ifindex Numeric interface index.
 * @param ifname  Output buffer of at least IF_NAMESIZE bytes.
 * @return @p ifname on success, or NULL with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/if_indextoname.html
 */
char *if_indextoname(unsigned int ifindex, char *ifname);

/**
 * @brief Free a list returned by if_nameindex().
 * @ingroup posix_option_group_networking
 * @param ptr List to free (must have been returned by if_nameindex()).
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/if_freenameindex.html
 */
void if_freenameindex(struct if_nameindex *ptr);

/**
 * @brief Return all network interfaces as a name-index array.
 * @ingroup posix_option_group_networking
 *
 * The returned array is terminated by an entry with @c if_index == 0 and
 * @c if_name == NULL.  Free with if_freenameindex().
 *
 * @return Allocated array on success, or NULL with errno set on failure.
 */
struct if_nameindex *if_nameindex(void);

/**
 * @brief Map a network interface name to its index.
 * @ingroup posix_option_group_networking
 * @param ifname Interface name string.
 * @return Interface index on success, or 0 with errno set on failure.
 */
unsigned int if_nametoindex(const char *ifname);


#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NET_IF_H_ */

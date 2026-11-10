/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>

struct getifaddrs_ctx {
	struct ifaddrs **tail;
	int err;
};

#if defined(CONFIG_NET_IPV4) || defined(CONFIG_NET_IPV6)
/* one allocation per list element; freeifaddrs() frees it through ifa */
struct ifaddrs_entry {
	struct ifaddrs ifa;
	struct sockaddr_storage addr;
	struct sockaddr_storage netmask;
	char name[IF_NAMESIZE];
};

static struct ifaddrs_entry *ifaddrs_entry_new(struct getifaddrs_ctx *ctx, struct net_if *iface)
{
	int ret = -1;
	struct ifaddrs_entry *e = calloc(1, sizeof(*e));

	if (e == NULL) {
		ctx->err = ENOMEM;
		return NULL;
	}

	e->ifa.ifa_name = e->name;
#ifdef CONFIG_NET_INTERFACE_NAME
	ret = net_if_get_name(iface, e->name, sizeof(e->name));
#endif
	if (ret < 0) {
		snprintf(e->name, sizeof(e->name), "if%d", net_if_get_by_iface(iface));
	}

	if (net_if_is_up(iface)) {
		e->ifa.ifa_flags |= IFF_UP | IFF_RUNNING;
	}

	*ctx->tail = &e->ifa;
	ctx->tail = &e->ifa.ifa_next;

	return e;
}

#endif /* CONFIG_NET_IPV4 || CONFIG_NET_IPV6 */

#ifdef CONFIG_NET_IPV4
static void getifaddrs_ipv4_cb(struct net_if *iface, struct net_if_addr *addr, void *user_data)
{
	struct getifaddrs_ctx *ctx = user_data;
	struct ifaddrs_entry *e;
	struct sockaddr_in *sin;
	struct net_in_addr mask;

	if ((ctx->err != 0) || (addr->address.family != NET_AF_INET)) {
		return;
	}

	e = ifaddrs_entry_new(ctx, iface);
	if (e == NULL) {
		return;
	}

	sin = (struct sockaddr_in *)&e->addr;
	sin->sin_family = AF_INET;
	memcpy(&sin->sin_addr, &addr->address.in_addr, sizeof(sin->sin_addr));
	e->ifa.ifa_addr = (struct sockaddr *)sin;

	sin = (struct sockaddr_in *)&e->netmask;
	sin->sin_family = AF_INET;
	mask = net_if_ipv4_get_netmask_by_addr(iface, &addr->address.in_addr);
	memcpy(&sin->sin_addr, &mask, sizeof(sin->sin_addr));
	e->ifa.ifa_netmask = (struct sockaddr *)sin;

	if (net_ipv4_is_addr_loopback(&addr->address.in_addr)) {
		e->ifa.ifa_flags |= IFF_LOOPBACK;
	}
}
#endif /* CONFIG_NET_IPV4 */

#ifdef CONFIG_NET_IPV6
static void getifaddrs_ipv6_cb(struct net_if *iface, struct net_if_addr *addr, void *user_data)
{
	struct getifaddrs_ctx *ctx = user_data;
	struct ifaddrs_entry *e;
	struct sockaddr_in6 *sin6;

	if ((ctx->err != 0) || (addr->address.family != NET_AF_INET6)) {
		return;
	}

	e = ifaddrs_entry_new(ctx, iface);
	if (e == NULL) {
		return;
	}

	sin6 = (struct sockaddr_in6 *)&e->addr;
	sin6->sin6_family = AF_INET6;
	memcpy(&sin6->sin6_addr, &addr->address.in6_addr, sizeof(sin6->sin6_addr));
	e->ifa.ifa_addr = (struct sockaddr *)sin6;

	if (net_ipv6_is_addr_loopback(&addr->address.in6_addr)) {
		e->ifa.ifa_flags |= IFF_LOOPBACK;
	}
}
#endif /* CONFIG_NET_IPV6 */

static void getifaddrs_iface_cb(struct net_if *iface, void *user_data)
{
#ifdef CONFIG_NET_IPV4
	net_if_ipv4_addr_foreach(iface, getifaddrs_ipv4_cb, user_data);
#endif
#ifdef CONFIG_NET_IPV6
	net_if_ipv6_addr_foreach(iface, getifaddrs_ipv6_cb, user_data);
#endif
#if !defined(CONFIG_NET_IPV4) && !defined(CONFIG_NET_IPV6)
	ARG_UNUSED(iface);
	ARG_UNUSED(user_data);
#endif
}

int getifaddrs(struct ifaddrs **ifap)
{
	struct getifaddrs_ctx ctx = {.tail = ifap};

	if (ifap == NULL) {
		errno = EINVAL;
		return -1;
	}

	*ifap = NULL;
	net_if_foreach(getifaddrs_iface_cb, &ctx);

	if (ctx.err != 0) {
		freeifaddrs(*ifap);
		*ifap = NULL;
		errno = ctx.err;
		return -1;
	}

	return 0;
}

void freeifaddrs(struct ifaddrs *ifa)
{
	while (ifa != NULL) {
		struct ifaddrs *next = ifa->ifa_next;

		free(ifa);
		ifa = next;
	}
}

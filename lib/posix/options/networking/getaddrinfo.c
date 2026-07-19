/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <string.h>

#include <netdb.h>

#include <zephyr/net/socket.h>
#include <zephyr/posix/net/conversion.h>

#if defined(CONFIG_DNS_RESOLVER_AI_MAX_ENTRIES)
#define AI_ARR_MAX CONFIG_DNS_RESOLVER_AI_MAX_ENTRIES
#else
#define AI_ARR_MAX 1
#endif

#if defined(CONFIG_NET_IP)
/* Sole consumer of this net-internal helper; forward-declared here rather than in a header. */
int try_resolve_literal_addr(const char *host, const char *service,
			     const struct zsock_addrinfo *hints, struct zsock_addrinfo *res);
#endif

/* slow path: a POSIX result entry with its own address and name storage */
struct posix_addrinfo_entry {
	struct addrinfo ai;
	struct sockaddr_storage addr;
	char canonname[DNS_MAX_NAME_SIZE + 1];
};

/* the two steps zsock_getaddrinfo() runs, into a result array the caller owns */
static int resolve(const char *host, const char *service, const struct zsock_addrinfo *hints,
		   struct zsock_addrinfo *res)
{
	int ret = EAI_FAIL;

#if defined(CONFIG_NET_IP)
	ret = try_resolve_literal_addr(host, service, hints, res);
#endif
#if defined(CONFIG_DNS_RESOLVER)
	if (ret != 0) {
		ret = z_zsock_getaddrinfo_internal(host, service, hints, res);
	}
#endif
#if !defined(CONFIG_NET_IP) && !defined(CONFIG_DNS_RESOLVER)
	ARG_UNUSED(host);
	ARG_UNUSED(service);
	ARG_UNUSED(hints);
	ARG_UNUSED(res);
#endif

	return ret;
}

/* slow path: copy the stack's result list into POSIX entries, one allocation for the lot */
static struct addrinfo *zephyr_addrinfo_to_posix(const struct zsock_addrinfo *zres)
{
	struct posix_addrinfo_entry *entries;
	const struct zsock_addrinfo *z;
	size_t n = 0;
	size_t i = 0;

	for (z = zres; z != NULL; z = z->ai_next) {
		n++;
	}

	entries = calloc(n, sizeof(*entries));
	if (entries == NULL) {
		return NULL;
	}

	for (z = zres; z != NULL; z = z->ai_next, i++) {
		struct posix_addrinfo_entry *e = &entries[i];
		size_t len = sizeof(e->addr);
		struct sockaddr *addr;

		zephyr_addrinfo_to_posix_fields(z, &e->ai);

		addr = zephyr_sockaddr_to_posix(z->ai_addr, z->ai_addrlen, (struct sockaddr *)&e->addr,
						&len);
		if (addr != (struct sockaddr *)&e->addr) {
			/* passed through unchanged: keep our own copy, the stack's array goes away */
			memcpy(&e->addr, addr, len);
		}
		e->ai.ai_addr = (struct sockaddr *)&e->addr;
		e->ai.ai_addrlen = len;

		e->ai.ai_canonname = NULL;
		if (z->ai_canonname != NULL) {
			strncpy(e->canonname, z->ai_canonname, sizeof(e->canonname) - 1);
			e->ai.ai_canonname = e->canonname;
		}

		e->ai.ai_next = (i + 1 < n) ? &entries[i + 1].ai : NULL;
	}

	return &entries[0].ai;
}

int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints,
		struct addrinfo **res)
{
	struct zsock_addrinfo zhints;
	const struct zsock_addrinfo *zhints_p = posix_addrinfo_hints_to_zephyr(hints, &zhints);
	struct zsock_addrinfo *zres;
	int ret;

	/* The caller owns the result array; the stack only fills what it is handed. */
	zres = calloc(AI_ARR_MAX, sizeof(*zres));
	if (zres == NULL) {
		return EAI_MEMORY;
	}

	ret = resolve(host, service, zhints_p, zres);
	if (ret != 0) {
		free(zres);
		*res = NULL;
		return ret;
	}

	if (posix_addrinfo_layout_eq()) {
		/* fast path: the stack's array is the POSIX result */
		*res = (struct addrinfo *)zres;
		return 0;
	}

	*res = zephyr_addrinfo_to_posix(zres);
	free(zres);

	return (*res != NULL) ? 0 : EAI_MEMORY;
}

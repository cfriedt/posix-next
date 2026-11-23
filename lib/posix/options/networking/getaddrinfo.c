/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
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

/* the hosts database, into the caller's array */
int posix_hosts_lookup(const char *host, uint16_t port, const struct zsock_addrinfo *hints,
		       struct zsock_addrinfo *res, size_t max);

/* slow path: a POSIX result entry with its own address and name storage */
struct posix_addrinfo_entry {
	struct addrinfo ai;
	struct sockaddr_storage addr;
	char canonname[DNS_MAX_NAME_SIZE + 1];
};

/* a service is a port number; a name is EAI_SERVICE, as one no database lists */
static int service_port(const char *service, uint16_t *port)
{
	char *end;
	long n;

	*port = 0;
	if (service == NULL) {
		return 0;
	}
	n = strtol(service, &end, 10);
	if ((*service == '\0') || (*end != '\0') || (n < 0) || (n > UINT16_MAX)) {
		return EAI_SERVICE;
	}
	*port = (uint16_t)n;

	return 0;
}

/* an address literal, the hosts database, then Zephyr's resolver, into the caller's array */
static int resolve(const char *host, const char *service, const struct zsock_addrinfo *hints,
		   struct zsock_addrinfo *res)
{
	uint16_t port;
	int ret;

	ret = service_port(service, &port);
	if (ret != 0) {
		return ret;
	}

	ret = EAI_NONAME;
#if defined(CONFIG_NET_IP)
	ret = try_resolve_literal_addr(host, service, hints, res);
#endif
	if (ret == 0) {
		return 0;
	}
	/* a name must not be mistaken for an address literal */
	if ((hints != NULL) && ((hints->ai_flags & ZSOCK_AI_NUMERICHOST) != 0)) {
		return EAI_NONAME;
	}
	if (posix_hosts_lookup(host, port, hints, res, AI_ARR_MAX) == 0) {
		return 0;
	}
#if defined(CONFIG_DNS_RESOLVER)
	ret = z_zsock_getaddrinfo_internal(host, service, hints, res);
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

/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <zephyr/net/socket.h>

#define HOSTS_LINE_MAX 256

/* the caller's array of entries, filled in order */
struct hosts_result {
	struct zsock_addrinfo *ai;
	size_t max;
	size_t n;
	int socktype;
	int protocol;
	uint16_t port;
	bool canon;
};

static bool is_space(char c)
{
	return (c == ' ') || (c == '\t');
}

/*
 * The database is read a line at a time over a descriptor rather than through stdio: in a
 * user-mode build picolibc's fopen() allocates a kernel mutex object for the FILE and fclose()
 * only revokes the caller's access to it, so every open leaked a kernel heap object until
 * collection and a few lookups exhausted a small kernel heap.
 */
struct line_reader {
	int fd;
	size_t len;
	size_t pos;
	bool eof;
	char buf[HOSTS_LINE_MAX];
};

static bool line_reader_open(struct line_reader *lr, const char *path)
{
	*lr = (struct line_reader){.fd = open(path, O_RDONLY)};

	return lr->fd >= 0;
}

static void line_reader_close(struct line_reader *lr)
{
	(void)close(lr->fd);
}

/* the next line without its terminator, cut at the buffer's size; false at the end */
static bool line_next(struct line_reader *lr, char *line, size_t n)
{
	size_t out = 0;
	bool any = false;

	while (true) {
		if (lr->pos == lr->len) {
			ssize_t got;

			if (lr->eof) {
				break;
			}
			got = read(lr->fd, lr->buf, sizeof(lr->buf));
			if (got <= 0) {
				lr->eof = true;
				break;
			}
			lr->pos = 0;
			lr->len = (size_t)got;
		}
		while (lr->pos < lr->len) {
			char c = lr->buf[lr->pos++];

			any = true;
			if (c == '\n') {
				line[out] = '\0';
				return true;
			}
			if (out + 1 < n) {
				line[out++] = c;
			}
		}
	}
	line[out] = '\0';

	return any;
}

/* the next word of a line, NUL-terminated in place; NULL at the end */
static char *next_word(char **cursor)
{
	char *p = *cursor;
	char *word;

	while (is_space(*p)) {
		p++;
	}
	if (*p == '\0') {
		*cursor = p;
		return NULL;
	}
	word = p;
	while ((*p != '\0') && !is_space(*p)) {
		p++;
	}
	if (*p != '\0') {
		*p++ = '\0';
	}
	*cursor = p;

	return word;
}

static bool name_equal(const char *a, const char *b)
{
	for (; (*a != '\0') && (*b != '\0'); a++, b++) {
		char ca = *a;
		char cb = *b;

		if ((ca >= 'A') && (ca <= 'Z')) {
			ca = (char)(ca - 'A' + 'a');
		}
		if ((cb >= 'A') && (cb <= 'Z')) {
			cb = (char)(cb - 'A' + 'a');
		}
		if (ca != cb) {
			return false;
		}
	}
	/* a trailing dot on either side names the same host */
	if ((*a == '.') && (a[1] == '\0')) {
		a++;
	}
	if ((*b == '.') && (b[1] == '\0')) {
		b++;
	}

	return (*a == '\0') && (*b == '\0');
}

static void result_add(struct hosts_result *r, int family, const void *addr, const char *canon)
{
	struct zsock_addrinfo *ai;

	if (r->n >= r->max) {
		return;
	}

	ai = &r->ai[r->n];
	memset(ai, 0, sizeof(*ai));
	if (family == AF_INET) {
		struct net_sockaddr_in *sin = (struct net_sockaddr_in *)&ai->_ai_addr;

		sin->sin_family = NET_AF_INET;
		sin->sin_port = htons(r->port);
		memcpy(&sin->sin_addr, addr, sizeof(sin->sin_addr));
		ai->ai_addrlen = sizeof(*sin);
		ai->ai_family = NET_AF_INET;
	} else {
		struct net_sockaddr_in6 *sin6 = (struct net_sockaddr_in6 *)&ai->_ai_addr;

		sin6->sin6_family = NET_AF_INET6;
		sin6->sin6_port = htons(r->port);
		memcpy(&sin6->sin6_addr, addr, sizeof(sin6->sin6_addr));
		ai->ai_addrlen = sizeof(*sin6);
		ai->ai_family = NET_AF_INET6;
	}
	ai->ai_addr = (struct net_sockaddr *)&ai->_ai_addr;
	ai->ai_socktype = r->socktype;
	ai->ai_protocol = r->protocol;
	if (r->canon && (canon != NULL) && (canon[0] != '\0')) {
		strncpy(ai->_ai_canonname, canon, sizeof(ai->_ai_canonname) - 1);
		ai->ai_canonname = ai->_ai_canonname;
	}
	if (r->n > 0) {
		r->ai[r->n - 1].ai_next = ai;
	}
	r->n++;
}

/* every line of the hosts database naming the host, in the family asked for */
int posix_hosts_lookup(const char *host, uint16_t port, const struct zsock_addrinfo *hints,
		       struct zsock_addrinfo *res, size_t max)
{
	struct hosts_result r = {
		.ai = res,
		.max = max,
		.socktype = NET_SOCK_STREAM,
		.port = port,
	};
	char line[HOSTS_LINE_MAX];
	int family = AF_UNSPEC;
	struct line_reader lr;

	if ((host == NULL) || (host[0] == '\0') || (max == 0)) {
		return EAI_NONAME;
	}
	if (hints != NULL) {
		r.canon = (hints->ai_flags & ZSOCK_AI_CANONNAME) != 0;
		family = hints->ai_family;
		if (hints->ai_socktype != 0) {
			r.socktype = hints->ai_socktype;
		}
		r.protocol = hints->ai_protocol;
	}
	if (r.protocol == 0) {
		r.protocol = (r.socktype == NET_SOCK_DGRAM) ? NET_IPPROTO_UDP : NET_IPPROTO_TCP;
	}

	if (!line_reader_open(&lr, "/etc/hosts")) {
		return EAI_NONAME;
	}

	while ((r.n < r.max) && line_next(&lr, line, sizeof(line))) {
		unsigned char buf[sizeof(struct in6_addr)];
		char *cursor = line;
		char *addr, *name, *canon = NULL;
		bool match = false;
		int af;

		line[strcspn(line, "#\r")] = '\0';
		addr = next_word(&cursor);
		if (addr == NULL) {
			continue;
		}
		while ((name = next_word(&cursor)) != NULL) {
			if (canon == NULL) {
				canon = name;
			}
			if (name_equal(name, host)) {
				match = true;
			}
		}
		if (!match) {
			continue;
		}
		if (inet_pton(AF_INET, addr, buf) == 1) {
			af = AF_INET;
		} else if (inet_pton(AF_INET6, addr, buf) == 1) {
			af = AF_INET6;
		} else {
			continue;
		}
		if ((family != AF_UNSPEC) && (family != af)) {
			continue;
		}
		result_add(&r, af, buf, canon);
	}
	line_reader_close(&lr);

	return (r.n > 0) ? 0 : EAI_NONAME;
}

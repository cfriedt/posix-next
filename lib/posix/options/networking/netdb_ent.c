/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <zephyr/sys/util.h>

#define NETDB_LINE_MAX 256

struct netdb_file {
	const char *path;
	FILE *fp;
	bool stayopen;
	char line[NETDB_LINE_MAX];
};

static struct netdb_file hosts_db = {.path = "/etc/hosts"};
static struct netdb_file networks_db = {.path = "/etc/networks"};
static struct netdb_file protocols_db = {.path = "/etc/protocols"};
static struct netdb_file services_db = {.path = "/etc/services"};

static struct hostent host_result;
static char *host_aliases[8];
static char host_addr_storage[4];
static char *host_addr_list[2];

static struct netent net_result;
static char *net_aliases[8];

static struct protoent proto_result;
static char *proto_aliases[8];

static struct servent serv_result;
static char *serv_aliases[8];

static bool is_comment_or_blank(const char *line)
{
	while (*line == ' ' || *line == '\t') {
		line++;
	}

	return (*line == '\0' || *line == '\n' || *line == '#');
}

static struct netdb_file *netdb_active;

static void netdb_release(struct netdb_file *db)
{
	if (db->fp != NULL) {
		fclose(db->fp);
		db->fp = NULL;
	}

	if (netdb_active == db) {
		netdb_active = NULL;
	}
}

static void netdb_release_others(struct netdb_file *db)
{
	if (hosts_db.fp != NULL && db != &hosts_db && !hosts_db.stayopen) {
		netdb_release(&hosts_db);
	}
	if (networks_db.fp != NULL && db != &networks_db && !networks_db.stayopen) {
		netdb_release(&networks_db);
	}
	if (protocols_db.fp != NULL && db != &protocols_db && !protocols_db.stayopen) {
		netdb_release(&protocols_db);
	}
	if (services_db.fp != NULL && db != &services_db && !services_db.stayopen) {
		netdb_release(&services_db);
	}
}

static FILE *netdb_open(struct netdb_file *db)
{
	if (db->fp != NULL) {
		return db->fp;
	}

	netdb_release_others(db);
	db->fp = fopen(db->path, "r");
	if (db->fp != NULL) {
		netdb_active = db;
	}

	return db->fp;
}

static void netdb_close(struct netdb_file *db)
{
	if (db->fp != NULL && !db->stayopen) {
		netdb_release(db);
	}
}

static void netdb_rewind(struct netdb_file *db)
{
	if (netdb_open(db) == NULL) {
		return;
	}

	rewind(db->fp);
}

static char *skip_ws(char *p)
{
	while (*p == ' ' || *p == '\t') {
		p++;
	}

	return p;
}

static void trim_newline(char *p)
{
	char *nl = strchr(p, '\n');

	if (nl != NULL) {
		*nl = '\0';
	}
}

static bool parse_hosts_line(char *line, struct hostent *he)
{
	char *addr;
	char *name;
	char *aliases;
	int ret;

	addr = skip_ws(line);
	if (*addr == '\0') {
		return false;
	}

	name = strchr(addr, ' ');
	if (name == NULL) {
		name = strchr(addr, '\t');
	}
	if (name == NULL) {
		return false;
	}

	*name++ = '\0';
	name = skip_ws(name);
	if (*name == '\0') {
		return false;
	}

	aliases = strchr(name, ' ');
	if (aliases == NULL) {
		aliases = strchr(name, '\t');
	}
	if (aliases != NULL) {
		*aliases++ = '\0';
		aliases = skip_ws(aliases);
	} else {
		aliases = NULL;
	}

	ret = inet_pton(AF_INET, addr, host_addr_storage);
	if (ret != 1) {
		return false;
	}

	host_addr_list[0] = host_addr_storage;
	host_addr_list[1] = NULL;

	he->h_name = name;
	he->h_aliases = host_aliases;
	he->h_addrtype = AF_INET;
	he->h_length = sizeof(host_addr_storage);
	he->h_addr_list = host_addr_list;

	host_aliases[0] = NULL;
	if (aliases != NULL && *aliases != '\0') {
		int i = 0;
		char *tok = aliases;

		while (tok != NULL && i < (int)ARRAY_SIZE(host_aliases) - 1) {
			host_aliases[i++] = tok;
			tok = strchr(tok, ' ');
			if (tok == NULL) {
				tok = strchr(host_aliases[i - 1], '\t');
			}
			if (tok != NULL) {
				*tok++ = '\0';
				tok = skip_ws(tok);
			}
		}
		host_aliases[i] = NULL;
	}

	return true;
}

struct hostent *gethostent(void)
{
	struct netdb_file *db = &hosts_db;

	if (netdb_open(db) == NULL) {
		return NULL;
	}

	while (fgets(db->line, sizeof(db->line), db->fp) != NULL) {
		trim_newline(db->line);
		if (is_comment_or_blank(db->line)) {
			continue;
		}

		if (parse_hosts_line(db->line, &host_result)) {
			if (!db->stayopen) {
				fclose(db->fp);
				db->fp = NULL;
			}
			return &host_result;
		}
	}

	netdb_close(db);
	return NULL;
}

void sethostent(int stayopen)
{
	hosts_db.stayopen = stayopen != 0;
	netdb_rewind(&hosts_db);
}

void endhostent(void)
{
	netdb_release(&hosts_db);
	hosts_db.stayopen = false;
}

static bool parse_networks_line(char *line, struct netent *ne)
{
	char *name;
	char *netstr;
	unsigned long net;
	char *aliases;
	int i = 0;

	name = skip_ws(line);
	if (*name == '\0') {
		return false;
	}

	netstr = strchr(name, ' ');
	if (netstr == NULL) {
		netstr = strchr(name, '\t');
	}
	if (netstr == NULL) {
		return false;
	}

	*netstr++ = '\0';
	netstr = skip_ws(netstr);
	if (*netstr == '\0') {
		return false;
	}

	aliases = strchr(netstr, ' ');
	if (aliases == NULL) {
		aliases = strchr(netstr, '\t');
	}
	if (aliases != NULL) {
		*aliases++ = '\0';
		aliases = skip_ws(aliases);
	} else {
		aliases = NULL;
	}

	net = strtoul(netstr, NULL, 10);

	ne->n_name = name;
	ne->n_addrtype = AF_INET;
	ne->n_net = (uint32_t)net;
	ne->n_aliases = net_aliases;
	net_aliases[0] = NULL;

	if (aliases != NULL && *aliases != '\0') {
		char *tok = aliases;

		while (tok != NULL && i < (int)ARRAY_SIZE(net_aliases) - 1) {
			net_aliases[i++] = tok;
			tok = strchr(tok, ' ');
			if (tok == NULL) {
				tok = strchr(net_aliases[i - 1], '\t');
			}
			if (tok != NULL) {
				*tok++ = '\0';
				tok = skip_ws(tok);
			}
		}
		net_aliases[i] = NULL;
	}

	return true;
}

static struct netent *lookup_network_fallback(const char *name, uint32_t net, bool match_net)
{
	static const struct {
		const char *n_name;
		uint32_t n_net;
	} fallback[] = {
		{ "loopback", 127U },
	};

	for (size_t i = 0; i < ARRAY_SIZE(fallback); i++) {
		if (name != NULL && strcmp(fallback[i].n_name, name) != 0) {
			continue;
		}

		if (match_net && fallback[i].n_net != net) {
			continue;
		}

		net_result.n_name = (char *)fallback[i].n_name;
		net_result.n_addrtype = AF_INET;
		net_result.n_net = fallback[i].n_net;
		net_result.n_aliases = net_aliases;
		net_aliases[0] = NULL;
		return &net_result;
	}

	return NULL;
}

static struct netent *scan_networks(const char *name, uint32_t net, bool match_net)
{
	struct netdb_file *db = &networks_db;
	struct netent *result = NULL;

	netdb_rewind(db);
	if (db->fp != NULL) {
		while (fgets(db->line, sizeof(db->line), db->fp) != NULL) {
			trim_newline(db->line);
			if (is_comment_or_blank(db->line)) {
				continue;
			}

			if (!parse_networks_line(db->line, &net_result)) {
				continue;
			}

			if (name != NULL && strcmp(net_result.n_name, name) != 0) {
				continue;
			}

			if (match_net && net_result.n_net != net) {
				continue;
			}

			result = &net_result;
			break;
		}

		if (!db->stayopen) {
			fclose(db->fp);
			db->fp = NULL;
		}
	}

	if (result == NULL) {
		result = lookup_network_fallback(name, net, match_net);
	}

	return result;
}

struct netent *getnetbyaddr(uint32_t net, int type)
{
	if (type != AF_INET) {
		return NULL;
	}

	return scan_networks(NULL, net, true);
}

struct netent *getnetbyname(const char *name)
{
	if (name == NULL) {
		return NULL;
	}

	return scan_networks(name, 0, false);
}

struct netent *getnetent(void)
{
	struct netdb_file *db = &networks_db;

	if (netdb_open(db) == NULL) {
		return NULL;
	}

	while (fgets(db->line, sizeof(db->line), db->fp) != NULL) {
		trim_newline(db->line);
		if (is_comment_or_blank(db->line)) {
			continue;
		}

		if (parse_networks_line(db->line, &net_result)) {
			if (!db->stayopen) {
				fclose(db->fp);
				db->fp = NULL;
			}
			return &net_result;
		}
	}

	netdb_close(db);
	return NULL;
}

void setnetent(int stayopen)
{
	networks_db.stayopen = stayopen != 0;
	netdb_rewind(&networks_db);
}

void endnetent(void)
{
	netdb_release(&networks_db);
	networks_db.stayopen = false;
}

static bool parse_protocols_line(char *line, struct protoent *pe)
{
	char *name;
	char *numstr;
	char *aliases;
	unsigned long num;
	int i = 0;

	name = skip_ws(line);
	if (*name == '\0') {
		return false;
	}

	numstr = strchr(name, ' ');
	if (numstr == NULL) {
		numstr = strchr(name, '\t');
	}
	if (numstr == NULL) {
		return false;
	}

	*numstr++ = '\0';
	numstr = skip_ws(numstr);
	if (*numstr == '\0') {
		return false;
	}

	aliases = strchr(numstr, ' ');
	if (aliases == NULL) {
		aliases = strchr(numstr, '\t');
	}
	if (aliases != NULL) {
		*aliases++ = '\0';
		aliases = skip_ws(aliases);
	} else {
		aliases = NULL;
	}

	num = strtoul(numstr, NULL, 10);

	pe->p_name = name;
	pe->p_proto = (int)num;
	pe->p_aliases = proto_aliases;
	proto_aliases[0] = NULL;

	if (aliases != NULL && *aliases != '\0') {
		char *tok = aliases;

		while (tok != NULL && i < (int)ARRAY_SIZE(proto_aliases) - 1) {
			proto_aliases[i++] = tok;
			tok = strchr(tok, ' ');
			if (tok == NULL) {
				tok = strchr(proto_aliases[i - 1], '\t');
			}
			if (tok != NULL) {
				*tok++ = '\0';
				tok = skip_ws(tok);
			}
		}
		proto_aliases[i] = NULL;
	}

	return true;
}

static struct protoent *scan_protocols(const char *name, int proto, bool match_proto)
{
	struct netdb_file *db = &protocols_db;
	struct protoent *result = NULL;

	netdb_rewind(db);
	if (db->fp == NULL) {
		return NULL;
	}

	while (fgets(db->line, sizeof(db->line), db->fp) != NULL) {
		trim_newline(db->line);
		if (is_comment_or_blank(db->line)) {
			continue;
		}

		if (!parse_protocols_line(db->line, &proto_result)) {
			continue;
		}

		if (name != NULL && strcmp(proto_result.p_name, name) != 0) {
			continue;
		}

		if (match_proto && proto_result.p_proto != proto) {
			continue;
		}

		result = &proto_result;
		break;
	}

	if (!db->stayopen) {
		fclose(db->fp);
		db->fp = NULL;
	}

	return result;
}

struct protoent *getprotobyname(const char *name)
{
	if (name == NULL) {
		return NULL;
	}

	return scan_protocols(name, 0, false);
}

struct protoent *getprotobynumber(int proto)
{
	return scan_protocols(NULL, proto, true);
}

struct protoent *getprotoent(void)
{
	struct netdb_file *db = &protocols_db;

	if (netdb_open(db) == NULL) {
		return NULL;
	}

	while (fgets(db->line, sizeof(db->line), db->fp) != NULL) {
		trim_newline(db->line);
		if (is_comment_or_blank(db->line)) {
			continue;
		}

		if (parse_protocols_line(db->line, &proto_result)) {
			if (!db->stayopen) {
				fclose(db->fp);
				db->fp = NULL;
			}
			return &proto_result;
		}
	}

	netdb_close(db);
	return NULL;
}

void setprotoent(int stayopen)
{
	protocols_db.stayopen = stayopen != 0;
	netdb_rewind(&protocols_db);
}

void endprotoent(void)
{
	netdb_release(&protocols_db);
	protocols_db.stayopen = false;
}

static bool parse_services_line(char *line, struct servent *se)
{
	char *name;
	char *portproto;
	char *aliases;
	unsigned long port;
	char *slash;
	char *proto;
	int i = 0;

	name = skip_ws(line);
	if (*name == '\0') {
		return false;
	}

	portproto = strchr(name, ' ');
	if (portproto == NULL) {
		portproto = strchr(name, '\t');
	}
	if (portproto == NULL) {
		return false;
	}

	*portproto++ = '\0';
	portproto = skip_ws(portproto);
	if (*portproto == '\0') {
		return false;
	}

	aliases = strchr(portproto, ' ');
	if (aliases == NULL) {
		aliases = strchr(portproto, '\t');
	}
	if (aliases != NULL) {
		*aliases++ = '\0';
		aliases = skip_ws(aliases);
	} else {
		aliases = NULL;
	}

	slash = strchr(portproto, '/');
	if (slash == NULL) {
		return false;
	}

	*slash++ = '\0';
	port = strtoul(portproto, NULL, 10);
	proto = slash;

	se->s_name = name;
	se->s_port = htons((uint16_t)port);
	se->s_proto = proto;
	se->s_aliases = serv_aliases;
	serv_aliases[0] = NULL;

	if (aliases != NULL && *aliases != '\0') {
		char *tok = aliases;

		while (tok != NULL && i < (int)ARRAY_SIZE(serv_aliases) - 1) {
			serv_aliases[i++] = tok;
			tok = strchr(tok, ' ');
			if (tok == NULL) {
				tok = strchr(serv_aliases[i - 1], '\t');
			}
			if (tok != NULL) {
				*tok++ = '\0';
				tok = skip_ws(tok);
			}
		}
		serv_aliases[i] = NULL;
	}

	return true;
}

static struct servent *scan_services(const char *name, int port, const char *proto)
{
	struct netdb_file *db = &services_db;
	struct servent *result = NULL;

	netdb_rewind(db);
	if (db->fp == NULL) {
		return NULL;
	}

	while (fgets(db->line, sizeof(db->line), db->fp) != NULL) {
		trim_newline(db->line);
		if (is_comment_or_blank(db->line)) {
			continue;
		}

		if (!parse_services_line(db->line, &serv_result)) {
			continue;
		}

		if (name != NULL && strcmp(serv_result.s_name, name) != 0) {
			continue;
		}

		if (name == NULL && serv_result.s_port != port) {
			continue;
		}

		if (proto != NULL && strcmp(serv_result.s_proto, proto) != 0) {
			continue;
		}

		result = &serv_result;
		break;
	}

	if (!db->stayopen) {
		fclose(db->fp);
		db->fp = NULL;
	}

	return result;
}

struct servent *getservbyname(const char *name, const char *proto)
{
	if (name == NULL) {
		return NULL;
	}

	return scan_services(name, 0, proto);
}

struct servent *getservbyport(int port, const char *proto)
{
	return scan_services(NULL, port, proto);
}

struct servent *getservent(void)
{
	struct netdb_file *db = &services_db;

	if (netdb_open(db) == NULL) {
		return NULL;
	}

	while (fgets(db->line, sizeof(db->line), db->fp) != NULL) {
		trim_newline(db->line);
		if (is_comment_or_blank(db->line)) {
			continue;
		}

		if (parse_services_line(db->line, &serv_result)) {
			if (!db->stayopen) {
				fclose(db->fp);
				db->fp = NULL;
			}
			return &serv_result;
		}
	}

	netdb_close(db);
	return NULL;
}

void setservent(int stayopen)
{
	services_db.stayopen = stayopen != 0;
	netdb_rewind(&services_db);
}

void endservent(void)
{
	netdb_release(&services_db);
	services_db.stayopen = false;
}

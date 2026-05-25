/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX network database operations (<netdb.h>)
 *
 * Provides hostname and service resolution (getaddrinfo/getnameinfo) and the
 * legacy host/network/protocol/service database functions.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netdb.h.html">
 *      POSIX.1-2017 &lt;netdb.h&gt;</a>
 *
 * @defgroup posix_netdb POSIX Network Database
 * @ingroup posix_option_group_networking
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETDB_H_
#define ZEPHYR_INCLUDE_POSIX_NETDB_H_

#include <zephyr/net/socket.h>

#ifndef NI_MAXSERV
/** @brief Reasonable buffer size for getnameinfo() service names. */
#define NI_MAXSERV 32
#endif

/** @brief Flags argument to getaddrinfo() contained an invalid value. */
#define EAI_BADFLAGS DNS_EAI_BADFLAGS
/** @brief The name does not resolve for the supplied parameters. */
#define EAI_NONAME   DNS_EAI_NONAME
/** @brief Temporary failure in name resolution. */
#define EAI_AGAIN    DNS_EAI_AGAIN
/** @brief Non-recoverable failure in name resolution. */
#define EAI_FAIL     DNS_EAI_FAIL
/** @brief The requested name is valid but has no address data. */
#define EAI_NODATA   DNS_EAI_NODATA
/** @brief Memory allocation failure. */
#define EAI_MEMORY   DNS_EAI_MEMORY
/** @brief A system error occurred; errno is set. */
#define EAI_SYSTEM   DNS_EAI_SYSTEM
/** @brief Requested service not available for the given socket type. */
#define EAI_SERVICE  DNS_EAI_SERVICE
/** @brief Socket type not supported. */
#define EAI_SOCKTYPE DNS_EAI_SOCKTYPE
/** @brief Address family not supported. */
#define EAI_FAMILY   DNS_EAI_FAMILY
/** @brief Output buffer overflow. */
#define EAI_OVERFLOW DNS_EAI_OVERFLOW

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Host entry returned by gethostbyname() (legacy). */
struct hostent {
	char *h_name;       /**< Official name of the host. */
	char **h_aliases;   /**< NULL-terminated list of alternate names. */
	int h_addrtype;     /**< Address type (AF_INET, etc.). */
	int h_length;       /**< Length of each address in bytes. */
	char **h_addr_list; /**< NULL-terminated list of addresses. */
};

/** @brief Network entry returned by getnetbyname() (legacy). */
struct netent {
	char *n_name;     /**< Official network name. */
	char **n_aliases; /**< NULL-terminated list of alternate names. */
	int n_addrtype;   /**< Address type (AF_INET). */
	uint32_t n_net;   /**< Network number (host byte order). */
};

/** @brief Protocol entry returned by getprotobyname() (legacy). */
struct protoent {
	char *p_name;     /**< Official protocol name. */
	char **p_aliases; /**< NULL-terminated list of alternate names. */
	int p_proto;      /**< Protocol number. */
};

/** @brief Service entry returned by getservbyname() (legacy). */
struct servent {
	char *s_name;     /**< Official service name. */
	char **s_aliases; /**< NULL-terminated list of alternate names. */
	int s_port;       /**< Port number (network byte order). */
	char *s_proto;    /**< Protocol to use ("tcp" or "udp"). */
};

/** @brief Address info structure (alias for zsock_addrinfo). */
#define addrinfo zsock_addrinfo

/** @brief Close the hosts database (legacy, no-op on most systems). */
void endhostent(void);
/** @brief Close the networks database (legacy, no-op on most systems). */
void endnetent(void);
/** @brief Close the protocols database (legacy). */
void endprotoent(void);
/** @brief Close the services database (legacy). */
void endservent(void);

/**
 * @brief Free addrinfo list returned by getaddrinfo().
 * @param ai Linked list to free.
 */
void freeaddrinfo(struct zsock_addrinfo *ai);

/**
 * @brief Return a string describing a getaddrinfo() error code.
 * @param errcode Error code returned by getaddrinfo().
 * @return Pointer to a description string.
 */
const char *gai_strerror(int errcode);

/**
 * @brief Translate a hostname and service to a list of socket addresses.
 * @param host    Hostname or numeric address string, or NULL.
 * @param service Service name or numeric port string, or NULL.
 * @param hints   Criteria for address selection, or NULL for defaults.
 * @param res     Output: linked list of matching addresses.
 * @return 0 on success, or a non-zero EAI_* error code.
 */
int getaddrinfo(const char *host, const char *service, const struct zsock_addrinfo *hints,
		struct zsock_addrinfo **res);

/**
 * @brief Get the next entry from the hosts database (legacy, sequential).
 * @return Pointer to a static hostent on success, or NULL at end or on error.
 */
struct hostent *gethostent(void);

/**
 * @brief Translate a socket address to a hostname and service name.
 * @param addr     Socket address to look up.
 * @param addrlen  Size of @p addr.
 * @param host     Output buffer for the hostname, or NULL.
 * @param hostlen  Size of @p host.
 * @param serv     Output buffer for the service name, or NULL.
 * @param servlen  Size of @p serv.
 * @param flags    NI_* flags controlling the conversion.
 * @return 0 on success, or a non-zero EAI_* error code.
 */
int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
		char *serv, socklen_t servlen, int flags);

/**
 * @brief Look up a network by address (legacy).
 * @param net  Network number (host byte order).
 * @param type Address type (AF_INET).
 * @return Pointer to a static netent, or NULL on failure.
 */
struct netent *getnetbyaddr(uint32_t net, int type);

/**
 * @brief Look up a network by name (legacy).
 * @param name Network name.
 * @return Pointer to a static netent, or NULL on failure.
 */
struct netent *getnetbyname(const char *name);

/**
 * @brief Get the next entry from the networks database (legacy, sequential).
 * @return Pointer to a static netent, or NULL at end or on error.
 */
struct netent *getnetent(void);

/**
 * @brief Look up a protocol by name (legacy).
 * @param name Protocol name (e.g. "tcp").
 * @return Pointer to a static protoent, or NULL on failure.
 */
struct protoent *getprotobyname(const char *name);

/**
 * @brief Look up a protocol by number (legacy).
 * @param proto Protocol number.
 * @return Pointer to a static protoent, or NULL on failure.
 */
struct protoent *getprotobynumber(int proto);

/**
 * @brief Get the next entry from the protocols database (legacy, sequential).
 * @return Pointer to a static protoent, or NULL at end or on error.
 */
struct protoent *getprotoent(void);

/**
 * @brief Look up a service by name and protocol (legacy).
 * @param name  Service name (e.g. "http").
 * @param proto Protocol ("tcp", "udp", or NULL for any).
 * @return Pointer to a static servent, or NULL on failure.
 */
struct servent *getservbyname(const char *name, const char *proto);

/**
 * @brief Look up a service by port number and protocol (legacy).
 * @param port  Port number (network byte order).
 * @param proto Protocol ("tcp", "udp", or NULL for any).
 * @return Pointer to a static servent, or NULL on failure.
 */
struct servent *getservbyport(int port, const char *proto);

/**
 * @brief Get the next entry from the services database (legacy, sequential).
 * @return Pointer to a static servent, or NULL at end or on error.
 */
struct servent *getservent(void);

/**
 * @brief Open the hosts database (legacy).
 * @param stayopen Non-zero to keep the database connection open.
 */
void sethostent(int stayopen);

/**
 * @brief Open the networks database (legacy).
 * @param stayopen Non-zero to keep the database connection open.
 */
void setnetent(int stayopen);

/**
 * @brief Open the protocols database (legacy).
 * @param stayopen Non-zero to keep the database connection open.
 */
void setprotoent(int stayopen);

/**
 * @brief Open the services database (legacy).
 * @param stayopen Non-zero to keep the database connection open.
 */
void setservent(int stayopen);

/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_NETDB_H_ */

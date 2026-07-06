/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX network database operations (<netdb.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netdb.h.html">
 *      POSIX.1-2017 &lt;netdb.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETDB_H_
#define ZEPHYR_INCLUDE_POSIX_NETDB_H_

#include <stdint.h>

#include <sys/socket.h>

#include <zephyr/net/dns_resolve.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_ADDRINFO_DECLARED) || defined(__addrinfo_defined)) || defined(__DOXYGEN__)
/** @brief Address info structure returned by getaddrinfo(). */
struct addrinfo {
	struct addrinfo *ai_next;     /**< Pointer to next in list. */
	int              ai_flags;    /**< Input flags. */
	int              ai_family;   /**< Address family of socket. */
	int              ai_socktype; /**< Socket type. */
	int              ai_protocol; /**< Protocol of socket. */
	int              ai_eflags;   /**< Extra flags (implementation). */
	socklen_t        ai_addrlen;  /**< Length of socket address. */
	struct sockaddr *ai_addr;     /**< Socket address of socket. */
	char            *ai_canonname; /**< Canonical name of service location. */
/** @cond INTERNAL_HIDDEN */
	struct zsock_sockaddr _ai_addr;
	char             _ai_canonname[DNS_MAX_NAME_SIZE + 1];
/** @endcond */
};
#define _ADDRINFO_DECLARED
#define __addrinfo_defined
#endif

/** @brief Socket address is intended for bind(). */
#define AI_PASSIVE ZSOCK_AI_PASSIVE

/** @brief Return canonical name for host. */
#define AI_CANONNAME ZSOCK_AI_CANONNAME

/** @brief Host is specified as a numeric address string. */
#define AI_NUMERICHOST ZSOCK_AI_NUMERICHOST

/** @brief Return IPv4-mapped IPv6 addresses when appropriate. */
#define AI_V4MAPPED ZSOCK_AI_V4MAPPED

/** @brief Return both IPv6 and IPv4-mapped addresses. */
#define AI_ALL ZSOCK_AI_ALL

/** @brief Return addresses only for configured address families. */
#define AI_ADDRCONFIG ZSOCK_AI_ADDRCONFIG

/** @brief Service is specified as a numeric port string. */
#define AI_NUMERICSERV ZSOCK_AI_NUMERICSERV

/** @brief Extended flags are present in ai_flags. */
#define AI_EXTFLAGS ZSOCK_AI_EXTFLAGS

/** @brief Flags argument to getaddrinfo() contained an invalid value. */
#define EAI_BADFLAGS DNS_EAI_BADFLAGS

/** @brief The name does not resolve for the supplied parameters. */
#define EAI_NONAME DNS_EAI_NONAME

/** @brief Temporary failure in name resolution. */
#define EAI_AGAIN DNS_EAI_AGAIN

/** @brief Non-recoverable failure in name resolution. */
#define EAI_FAIL DNS_EAI_FAIL

/** @brief The requested name is valid but has no address data. */
#define EAI_NODATA DNS_EAI_NODATA

/** @brief Memory allocation failure. */
#define EAI_MEMORY DNS_EAI_MEMORY

/** @brief A system error occurred; errno is set. */
#define EAI_SYSTEM DNS_EAI_SYSTEM

/** @brief Requested service not available for the given socket type. */
#define EAI_SERVICE DNS_EAI_SERVICE

/** @brief Socket type not supported. */
#define EAI_SOCKTYPE DNS_EAI_SOCKTYPE

/** @brief Address family not supported. */
#define EAI_FAMILY DNS_EAI_FAMILY

/** @brief Output buffer overflow. */
#define EAI_OVERFLOW DNS_EAI_OVERFLOW

/** @brief Maximum buffer size for getnameinfo() host names. */
#define NI_MAXHOST ZSOCK_NI_MAXHOST

/** @brief Maximum buffer size for getnameinfo() service names. */
#define NI_MAXSERV ZSOCK_NI_MAXSERV

/** @brief Return a numeric host address in @p host. */
#define NI_NUMERICHOST ZSOCK_NI_NUMERICHOST

/** @brief Return the numeric port number in @p serv. */
#define NI_NUMERICSERV ZSOCK_NI_NUMERICSERV

/** @brief Do not return the fully qualified domain name in @p host. */
#define NI_NOFQDN ZSOCK_NI_NOFQDN

/** @brief Return an error if the host name cannot be located. */
#define NI_NAMEREQD ZSOCK_NI_NAMEREQD

/** @brief Look up the service name for a datagram socket. */
#define NI_DGRAM ZSOCK_NI_DGRAM

#if !(defined(_HOSTENT_DECLARED) || defined(__hostent_defined)) || defined(__DOXYGEN__)
/** @brief Host database entry. */
struct hostent {
	char  *h_name;       /**< Official name of the host. */
	char **h_aliases;    /**< Alternate host names (null-terminated). */
	int    h_addrtype;   /**< Address type. */
	int    h_length;     /**< Length of the address in bytes. */
	char **h_addr_list;  /**< Network addresses for the host (null-terminated). */
};
#define _HOSTENT_DECLARED
#define __hostent_defined
#endif

#if !(defined(_NETENT_DECLARED) || defined(__netent_defined)) || defined(__DOXYGEN__)
/** @brief Network database entry. */
struct netent {
	char    *n_name;    /**< Official name of the network. */
	char   **n_aliases; /**< Alternate network names (null-terminated). */
	int      n_addrtype; /**< Address type of the network. */
	uint32_t n_net;     /**< Network number in host byte order. */
};
#define _NETENT_DECLARED
#define __netent_defined
#endif

#if !(defined(_PROTOENT_DECLARED) || defined(__protoent_defined)) || defined(__DOXYGEN__)
/** @brief Protocol database entry. */
struct protoent {
	char  *p_name;    /**< Official name of the protocol. */
	char **p_aliases; /**< Alternate protocol names (null-terminated). */
	int    p_proto;   /**< Protocol number. */
};
#define _PROTOENT_DECLARED
#define __protoent_defined
#endif

#if !(defined(_SERVENT_DECLARED) || defined(__servent_defined)) || defined(__DOXYGEN__)
/** @brief Service database entry. */
struct servent {
	char  *s_name;    /**< Official service name. */
	char **s_aliases; /**< Alternate service names (null-terminated). */
	int    s_port;    /**< Port number in network byte order. */
	char  *s_proto;   /**< Protocol to use ("tcp" or "udp"). */
};
#define _SERVENT_DECLARED
#define __servent_defined
#endif

/**
 * @brief Close the hosts database.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endhostent.html
 */
void endhostent(void);

/**
 * @brief Close the networks database.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endnetent.html
 */
void endnetent(void);

/**
 * @brief Close the protocols database.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endprotoent.html
 */
void endprotoent(void);

/**
 * @brief Close the services database.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endservent.html
 */
void endservent(void);

/**
 * @brief Free addrinfo list returned by getaddrinfo().
 * @param ai Linked list to free.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/freeaddrinfo.html
 */
void freeaddrinfo(struct addrinfo *ai);

/**
 * @brief Return a string describing a getaddrinfo() error code.
 * @param errcode Error code returned by getaddrinfo().
 * @return Pointer to a description string.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gai_strerror.html
 */
const char *gai_strerror(int errcode);

/**
 * @brief Translate a hostname and service to a list of socket addresses.
 * @param host    Hostname or numeric address string, or NULL.
 * @param service Service name or numeric port string, or NULL.
 * @param hints   Criteria for address selection, or NULL for defaults.
 * @param res     Output: linked list of matching addresses.
 * @return 0 on success, or a non-zero EAI_* error code.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getaddrinfo.html
 */
int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints,
		struct addrinfo **res);

/**
 * @brief Get the next entry from the hosts database.
 * @return Pointer to a host database entry, or NULL on end-of-file or error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gethostent.html
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
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnameinfo.html
 */
int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
		char *serv, socklen_t servlen, int flags);

/**
 * @brief Look up a network by address.
 * @param net  Network number in host byte order.
 * @param type Address type (for example AF_INET).
 * @return Pointer to a network database entry, or NULL if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetbyaddr.html
 */
struct netent *getnetbyaddr(uint32_t net, int type);

/**
 * @brief Look up a network by name.
 * @param name Network name.
 * @return Pointer to a network database entry, or NULL if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetbyname.html
 */
struct netent *getnetbyname(const char *name);

/**
 * @brief Get the next entry from the networks database.
 * @return Pointer to a network database entry, or NULL on end-of-file or error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetent.html
 */
struct netent *getnetent(void);

/**
 * @brief Look up a protocol by name.
 * @param name Protocol name.
 * @return Pointer to a protocol database entry, or NULL if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobyname.html
 */
struct protoent *getprotobyname(const char *name);

/**
 * @brief Look up a protocol by number.
 * @param proto Protocol number.
 * @return Pointer to a protocol database entry, or NULL if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobynumber.html
 */
struct protoent *getprotobynumber(int proto);

/**
 * @brief Get the next entry from the protocols database.
 * @return Pointer to a protocol database entry, or NULL on end-of-file or error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotoent.html
 */
struct protoent *getprotoent(void);

/**
 * @brief Look up a service by name and protocol.
 * @param name  Service name.
 * @param proto Protocol name (for example "tcp" or "udp"), or NULL for any.
 * @return Pointer to a service database entry, or NULL if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservbyname.html
 */
struct servent *getservbyname(const char *name, const char *proto);

/**
 * @brief Look up a service by port number and protocol.
 * @param port  Port number in network byte order.
 * @param proto Protocol name (for example "tcp" or "udp"), or NULL for any.
 * @return Pointer to a service database entry, or NULL if not found.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservbyport.html
 */
struct servent *getservbyport(int port, const char *proto);

/**
 * @brief Get the next entry from the services database.
 * @return Pointer to a service database entry, or NULL on end-of-file or error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservent.html
 */
struct servent *getservent(void);

/**
 * @brief Open the hosts database.
 * @param stayopen Non-zero to keep the database connection open.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sethostent.html
 */
void sethostent(int stayopen);

/**
 * @brief Open the networks database.
 * @param stayopen Non-zero to keep the database connection open.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setnetent.html
 */
void setnetent(int stayopen);

/**
 * @brief Open the protocols database.
 * @param stayopen Non-zero to keep the database connection open.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setprotoent.html
 */
void setprotoent(int stayopen);

/**
 * @brief Open the services database.
 * @param stayopen Non-zero to keep the database connection open.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setservent.html
 */
void setservent(int stayopen);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETDB_H_ */

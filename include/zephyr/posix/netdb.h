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
 * host/network/protocol/service database functions.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netdb.h.html">
 *      POSIX.1-2017 &lt;netdb.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_NETDB_H_
#define ZEPHYR_INCLUDE_POSIX_NETDB_H_

#include <stdint.h>

#include <zephyr/net/socket.h>
#include <zephyr/net/dns_resolve.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !(defined(_HOSTENT_DECLARED) || defined(__hostent_defined)) || defined(__DOXYGEN__)
/** @brief Host database entry. */
struct hostent {
	char *h_name;       /**< Official name of the host. */
	char **h_aliases;   /**< @c NULL-terminated list of alternate names. */
	int h_addrtype;     /**< Address type (@ref AF_INET, etc.). */
	int h_length;       /**< Length of each address in bytes. */
	char **h_addr_list; /**< @c NULL-terminated list of addresses. */
};
#define _HOSTENT_DECLARED
#define __hostent_defined
#endif

#if !(defined(_NETENT_DECLARED) || defined(__netent_defined)) || defined(__DOXYGEN__)
/** @brief Network database entry. */
struct netent {
	char *n_name;     /**< Official network name. */
	char **n_aliases; /**< @c NULL-terminated list of alternate names. */
	int n_addrtype;   /**< Address type (@ref AF_INET). */
	uint32_t n_net;   /**< Network number (host byte order). */
};
#define _NETENT_DECLARED
#define __netent_defined
#endif

#if !(defined(_PROTOENT_DECLARED) || defined(__protoent_defined)) || defined(__DOXYGEN__)
/** @brief Protocol database entry. */
struct protoent {
	char *p_name;     /**< Official protocol name. */
	char **p_aliases; /**< @c NULL-terminated list of alternate names. */
	int p_proto;      /**< Protocol number. */
};
#define _PROTOENT_DECLARED
#define __protoent_defined
#endif

#if !(defined(_SERVENT_DECLARED) || defined(__servent_defined)) || defined(__DOXYGEN__)
/** @brief Service database entry. */
struct servent {
	char *s_name;     /**< Official service name. */
	char **s_aliases; /**< @c NULL-terminated list of alternate names. */
	int s_port;       /**< Port number (network byte order). */
	char *s_proto;    /**< Protocol to use ("tcp" or "udp"). */
};
#define _SERVENT_DECLARED
#define __servent_defined
#endif

/** @brief Highest reserved Internet port number. */
#define IPPORT_RESERVED 1024

/*
 * required by addrinfo and several functions but absent in the spec
 * https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/netdb.h.html
 */
#if !defined(_SOCKLEN_T_DECLARED) && !defined(__socklen_t_defined)
typedef net_socklen_t socklen_t;
#define _SOCKLEN_T_DECLARED
#define __socklen_t_defined
#endif

#if !defined(_SA_FAMILY_T_DECLARED) && !defined(__sa_family_t_defined)
typedef net_sa_family_t sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
#endif

#if !defined(_SOCKADDR_DECLARED) && !defined(__sockaddr_defined)
struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};
#define _SOCKADDR_DECLARED
#define __sockaddr_defined
#endif

#if !(defined(_ADDRINFO_DECLARED) || defined(__addrinfo_defined)) || defined(__DOXYGEN__)
/** @brief Address info structure returned by @ref getaddrinfo. */
struct addrinfo {
	struct addrinfo *ai_next;      /**< Next entry in the linked list, or @c NULL. */
	int              ai_flags;     /**< Input @c AI_\* flags. */
	int              ai_family;    /**< Address family (@ref AF_INET, @ref AF_INET6, @ref AF_UNSPEC). */
	int              ai_socktype;  /**< Socket type (@ref SOCK_STREAM, @ref SOCK_DGRAM, ...). */
	int              ai_protocol;  /**< Protocol (@c IPPROTO_\*), or 0 for any. */
	/** @cond INTERNAL_HIDDEN */
	int              ai_eflags; /* for zephyr compatibility */
	/** INTERNAL_HIDDEN @endcond */
	socklen_t        ai_addrlen;   /**< Length of @c addrinfo.ai_addr in bytes. */
	struct sockaddr *ai_addr;      /**< Socket address for this entry. */
	char            *ai_canonname; /**< Canonical host name, or @c NULL. */
	/** @cond INTERNAL_HIDDEN */
	struct net_sockaddr_storage _ai_addr; /* for zephyr compatibility */
	char             _ai_canonname[DNS_MAX_NAME_SIZE + 1]; /* for zephyr compatibility */
	/** INTERNAL_HIDDEN @endcond */
};
#define _ADDRINFO_DECLARED
#define __addrinfo_defined
#endif

/* getaddrinfo() flags for @c addrinfo.ai_flags. */
/** @brief Return an address suitable for bind() (a passive open). */
#define AI_PASSIVE ZSOCK_AI_PASSIVE
/** @brief Request the canonical host name in @c addrinfo.ai_canonname. */
#define AI_CANONNAME ZSOCK_AI_CANONNAME
/** @brief Treat the host name as a numeric address string. */
#define AI_NUMERICHOST ZSOCK_AI_NUMERICHOST
/** @brief Treat the service name as a numeric port string. */
#define AI_NUMERICSERV ZSOCK_AI_NUMERICSERV
/** @brief Return IPv4-mapped IPv6 addresses when no IPv6 addresses are found. */
#define AI_V4MAPPED ZSOCK_AI_V4MAPPED
/** @brief With @ref AI_V4MAPPED, return both IPv6 and IPv4-mapped addresses. */
#define AI_ALL ZSOCK_AI_ALL
/** @brief Return addresses only for locally configured address families. */
#define AI_ADDRCONFIG ZSOCK_AI_ADDRCONFIG

/* getnameinfo() flags for the @p flags argument. */
/** @brief Do not return the fully qualified domain name. */
#define NI_NOFQDN ZSOCK_NI_NOFQDN
/** @brief Return the host address in numeric form. */
#define NI_NUMERICHOST ZSOCK_NI_NUMERICHOST
/** @brief Return an error if the host name cannot be located. */
#define NI_NAMEREQD ZSOCK_NI_NAMEREQD
/** @brief Return the service in numeric (port number) form. */
#define NI_NUMERICSERV ZSOCK_NI_NUMERICSERV
/** @brief Return the numeric form of the scope identifier. */
#define NI_NUMERICSCOPE 32
/** @brief Look up the service name for a datagram socket. */
#define NI_DGRAM ZSOCK_NI_DGRAM

/* Error values for @ref getaddrinfo and @ref getnameinfo. */
/** @brief Temporary failure in name resolution. */
#define EAI_AGAIN    DNS_EAI_AGAIN
/** @brief Flags argument to @ref getaddrinfo contained an invalid value. */
#define EAI_BADFLAGS DNS_EAI_BADFLAGS
/** @brief Non-recoverable failure in name resolution. */
#define EAI_FAIL     DNS_EAI_FAIL
/** @brief Address family not supported. */
#define EAI_FAMILY   DNS_EAI_FAMILY
/** @brief Memory allocation failure. */
#define EAI_MEMORY   DNS_EAI_MEMORY
/** @brief The name does not resolve for the supplied parameters. */
#define EAI_NONAME   DNS_EAI_NONAME
/** @brief Requested service not available for the given socket type. */
#define EAI_SERVICE  DNS_EAI_SERVICE
/** @brief Socket type not supported. */
#define EAI_SOCKTYPE DNS_EAI_SOCKTYPE
/** @brief A system error occurred; errno is set. */
#define EAI_SYSTEM   DNS_EAI_SYSTEM
/** @brief Output buffer overflow. */
#define EAI_OVERFLOW DNS_EAI_OVERFLOW

/* Non-POSIX extensions. */
#ifndef NI_MAXHOST
/** @brief Reasonable buffer size for @ref getnameinfo host names (extension). */
#define NI_MAXHOST 64
#endif
#ifndef NI_MAXSERV
/** @brief Reasonable buffer size for @ref getnameinfo service names (extension). */
#define NI_MAXSERV 32
#endif
/** @brief The requested name is valid but has no address data (obsolescent extension). */
#define EAI_NODATA   DNS_EAI_NODATA

/**
 * @brief Close the hosts database.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endhostent.html
 */
void endhostent(void);

/**
 * @brief Close the networks database.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endnetent.html
 */
void endnetent(void);

/**
 * @brief Close the protocols database.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endprotoent.html
 */
void endprotoent(void);

/**
 * @brief Close the services database.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endservent.html
 */
void endservent(void);

/**
 * @brief Free addrinfo list returned by @ref getaddrinfo.
 * @param ai Linked list to free.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/freeaddrinfo.html
 */
void freeaddrinfo(struct addrinfo *ai);

/**
 * @brief Return a string describing a @ref getaddrinfo error code.
 * @param errcode Error code returned by @ref getaddrinfo.
 * @return Pointer to a description string.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gai_strerror.html
 */
const char *gai_strerror(int errcode);

/**
 * @brief Translate a hostname and service to a list of socket addresses.
 * @param host    Hostname or numeric address string, or @c NULL.
 * @param service Service name or numeric port string, or @c NULL.
 * @param hints   Criteria for address selection, or @c NULL for defaults.
 * @param res     Output: linked list of matching addresses.
 * @return 0 on success, or a non-zero @c EAI_\* error code.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getaddrinfo.html
 */
int getaddrinfo(const char *host, const char *service, const struct addrinfo *hints,
		struct addrinfo **res);

/**
 * @brief Get the next entry from the hosts database.
 * @return Pointer to a static hostent on success, or @c NULL at end or on error.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gethostent.html
 */
struct hostent *gethostent(void);

/**
 * @brief Translate a socket address to a hostname and service name.
 * @param addr     Socket address to look up.
 * @param addrlen  Size of @p addr.
 * @param host     Output buffer for the hostname, or @c NULL.
 * @param hostlen  Size of @p host.
 * @param serv     Output buffer for the service name, or @c NULL.
 * @param servlen  Size of @p serv.
 * @param flags    NI_* flags controlling the conversion.
 * @return 0 on success, or a non-zero @c EAI_\* error code.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnameinfo.html
 */
int getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
		char *serv, socklen_t servlen, int flags);

/**
 * @brief Look up a network by address.
 * @param net  Network number (host byte order).
 * @param type Address type (@ref AF_INET).
 * @return Pointer to a static netent, or @c NULL on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetbyaddr.html
 */
struct netent *getnetbyaddr(uint32_t net, int type);

/**
 * @brief Look up a network by name.
 * @param name Network name.
 * @return Pointer to a static netent, or @c NULL on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetbyname.html
 */
struct netent *getnetbyname(const char *name);

/**
 * @brief Get the next entry from the networks database.
 * @return Pointer to a static netent, or @c NULL at end or on error.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetent.html
 */
struct netent *getnetent(void);

/**
 * @brief Look up a protocol by name.
 * @param name Protocol name (e.g. @c "tcp").
 * @return Pointer to a static protoent, or @c NULL on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobyname.html
 */
struct protoent *getprotobyname(const char *name);

/**
 * @brief Look up a protocol by number.
 * @param proto Protocol number.
 * @return Pointer to a static protoent, or @c NULL on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobynumber.html
 */
struct protoent *getprotobynumber(int proto);

/**
 * @brief Get the next entry from the protocols database.
 * @return Pointer to a static protoent, or @c NULL at end or on error.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotoent.html
 */
struct protoent *getprotoent(void);

/**
 * @brief Look up a service by name and protocol.
 * @param name  Service name (e.g. @c "http").
 * @param proto Protocol (@c "tcp", @c "udp", or @c NULL for any).
 * @return Pointer to a static servent, or @c NULL on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservbyname.html
 */
struct servent *getservbyname(const char *name, const char *proto);

/**
 * @brief Look up a service by port number and protocol.
 * @param port  Port number (network byte order).
 * @param proto Protocol (@c "tcp", @c "udp", or @c NULL for any).
 * @return Pointer to a static servent, or @c NULL on failure.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservbyport.html
 */
struct servent *getservbyport(int port, const char *proto);

/**
 * @brief Get the next entry from the services database.
 * @return Pointer to a static servent, or @c NULL at end or on error.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservent.html
 */
struct servent *getservent(void);

/**
 * @brief Open the hosts database.
 * @param stayopen Non-zero to keep the database connection open.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sethostent.html
 */
void sethostent(int stayopen);

/**
 * @brief Open the networks database.
 * @param stayopen Non-zero to keep the database connection open.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setnetent.html
 */
void setnetent(int stayopen);

/**
 * @brief Open the protocols database.
 * @param stayopen Non-zero to keep the database connection open.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setprotoent.html
 */
void setprotoent(int stayopen);

/**
 * @brief Open the services database.
 * @param stayopen Non-zero to keep the database connection open.
 * @ingroup posix_option_group_networking
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setservent.html
 */
void setservent(int stayopen);

#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_NETDB_H_ */

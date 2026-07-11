/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Network database operations (<netdb.h>)
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

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_IN_PORT_T_DECLARED) && !defined(__in_port_t_defined)
typedef uint16_t in_port_t;
#define _IN_PORT_T_DECLARED
#define __in_port_t_defined
#endif

#if !defined(_IN_ADDR_T_DECLARED) && !defined(__in_addr_t_defined)
typedef uint32_t in_addr_t;
#define _IN_ADDR_T_DECLARED
#define __in_addr_t_defined
#endif

#if !(defined(_HOSTENT_DECLARED) || defined(__hostent_defined)) || defined(__DOXYGEN__)
/** @brief Host database entry. */
struct hostent {
	/** @brief Official name of the host. */
	char *h_name;
	/** @brief Null-terminated list of alternative host names. */
	char **h_aliases;
	/** @brief Address type. */
	int h_addrtype;
	/** @brief Length of each address in bytes. */
	int h_length;
	/** @brief Null-terminated list of network addresses for the host. */
	char **h_addr_list;
};
#define _HOSTENT_DECLARED
#define __hostent_defined
#endif

#if !(defined(_NETENT_DECLARED) || defined(__netent_defined)) || defined(__DOXYGEN__)
/** @brief Network database entry. */
struct netent {
	/** @brief Official, fully-qualified name of the network. */
	char *n_name;
	/** @brief Null-terminated list of alternative network names. */
	char **n_aliases;
	/** @brief Address type of the network. */
	int n_addrtype;
	/** @brief Network number in host byte order. */
	uint32_t n_net;
};
#define _NETENT_DECLARED
#define __netent_defined
#endif

#if !(defined(_PROTOENT_DECLARED) || defined(__protoent_defined)) || defined(__DOXYGEN__)
/** @brief Protocol database entry. */
struct protoent {
	/** @brief Official name of the protocol. */
	char *p_name;
	/** @brief Null-terminated list of alternative protocol names. */
	char **p_aliases;
	/** @brief Protocol number. */
	int p_proto;
};
#define _PROTOENT_DECLARED
#define __protoent_defined
#endif

#if !(defined(_SERVENT_DECLARED) || defined(__servent_defined)) || defined(__DOXYGEN__)
/** @brief Service database entry. */
struct servent {
	/** @brief Official name of the service. */
	char *s_name;
	/** @brief Null-terminated list of alternative service names. */
	char **s_aliases;
	/** @brief Port number in network byte order. */
	int s_port;
	/** @brief Name of the protocol to use when contacting the service. */
	char *s_proto;
};
#define _SERVENT_DECLARED
#define __servent_defined
#endif

#if !defined(_SA_FAMILY_T_DECLARED) && !defined(__sa_family_t_defined)
typedef uint16_t sa_family_t;
#define _SA_FAMILY_T_DECLARED
#define __sa_family_t_defined
#endif

#if !defined(_SOCKLEN_T_DECLARED) && !defined(__socklen_t_defined)
typedef uint32_t socklen_t;
#define _SOCKLEN_T_DECLARED
#define __socklen_t_defined
#endif

#if !defined(_SOCKADDR_DECLARED) && !defined(__sockaddr_defined)
struct sockaddr {
	sa_family_t sa_family;
	char sa_data[];
};
#define _SOCKADDR_DECLARED
#define __sockaddr_defined
#endif

#if !(defined(_ADDRINFO_DECLARED) || defined(__addrinfo_defined)) || defined(__DOXYGEN__)
/**
 * @brief Address information structure.
 *
 * @note Field order matches @ref zsock_addrinfo so that a plain cast between
 *       the two struct types is valid when all named fields share the same
 *       offset and size.  POSIX does not mandate field order, so this
 *       deviation is conformant.  @c ai_eflags is a Zephyr extension used by
 *       the DNS resolver; POSIX code may safely ignore it.
 */
struct addrinfo {
	/** @brief Pointer to next entry in the list. */
	struct addrinfo *ai_next;
	/** @brief Input flags. */
	int ai_flags;
	/** @brief Address family of socket. */
	int ai_family;
	/** @brief Socket type. */
	int ai_socktype;
	/** @brief Protocol of socket. */
	int ai_protocol;
	/** @brief Extended flags (Zephyr extension, ignored by POSIX code). */
	int ai_eflags;
	/** @brief Length of socket address. */
	socklen_t ai_addrlen;
	/** @brief Socket address of socket. */
	struct sockaddr *ai_addr;
	/** @brief Canonical name of service location. */
	char *ai_canonname;
};
#define _ADDRINFO_DECLARED
#define __addrinfo_defined
#endif

/** @brief Highest reserved Internet port number. */
#define IPPORT_RESERVED 1023

/** @brief Socket address is intended for bind(). */
#define AI_PASSIVE     ZSOCK_AI_PASSIVE
/** @brief Request canonical name. */
#define AI_CANONNAME   ZSOCK_AI_CANONNAME
/** @brief Return numeric host address as name. */
#define AI_NUMERICHOST ZSOCK_AI_NUMERICHOST
/** @brief Inhibit service name resolution. */
#define AI_NUMERICSERV ZSOCK_AI_NUMERICSERV
/** @brief Return IPv4-mapped IPv6 addresses when no IPv6 addresses are found. */
#define AI_V4MAPPED    ZSOCK_AI_V4MAPPED
/** @brief Query for both IPv4 and IPv6 addresses. */
#define AI_ALL         ZSOCK_AI_ALL
/** @brief Limit address families to those configured on the local system. */
#define AI_ADDRCONFIG  ZSOCK_AI_ADDRCONFIG

/** @brief Return only the nodename portion of the FQDN for local hosts. */
#define NI_NOFQDN      ZSOCK_NI_NOFQDN
/** @brief Return the numeric form of the node address. */
#define NI_NUMERICHOST ZSOCK_NI_NUMERICHOST
/** @brief Return an error if the node name cannot be located. */
#define NI_NAMEREQD    ZSOCK_NI_NAMEREQD
/** @brief Return the numeric form of the service address. */
#define NI_NUMERICSERV ZSOCK_NI_NUMERICSERV
/** @brief Indicate that the service is a datagram service (@c SOCK_DGRAM). */
#define NI_DGRAM       ZSOCK_NI_DGRAM

/** @brief Recommended maximum host name buffer size for getnameinfo(). */
#define NI_MAXHOST     ZSOCK_NI_MAXHOST

/** @brief Temporary failure in name resolution. */
#define EAI_AGAIN      DNS_EAI_AGAIN
/** @brief Invalid flags value. */
#define EAI_BADFLAGS   DNS_EAI_BADFLAGS
/** @brief Non-recoverable failure in name resolution. */
#define EAI_FAIL       DNS_EAI_FAIL
/** @brief Address family not recognized or invalid address length. */
#define EAI_FAMILY     DNS_EAI_FAMILY
/** @brief Memory allocation failure. */
#define EAI_MEMORY     DNS_EAI_MEMORY
/** @brief Name does not resolve for the supplied parameters. */
#define EAI_NONAME     DNS_EAI_NONAME
/** @brief Service not recognized for the specified socket type. */
#define EAI_SERVICE    DNS_EAI_SERVICE
/** @brief Intended socket type not recognized. */
#define EAI_SOCKTYPE   DNS_EAI_SOCKTYPE
/** @brief System error; see @c errno. */
#define EAI_SYSTEM     DNS_EAI_SYSTEM
/** @brief Argument buffer overflow. */
#define EAI_OVERFLOW   DNS_EAI_OVERFLOW

/**
 * @brief Close the host database.
 *
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endhostent.html
 */
void endhostent(void);

/**
 * @brief Close the network database.
 *
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endnetent.html
 */
void endnetent(void);

/**
 * @brief Close the protocol database.
 *
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endprotoent.html
 */
void endprotoent(void);

/**
 * @brief Close the services database.
 *
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/endservent.html
 */
void endservent(void);

/**
 * @brief Free a list returned by getaddrinfo().
 *
 * @param ai Linked list to free.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/freeaddrinfo.html
 */
void freeaddrinfo(struct addrinfo *ai);

/**
 * @brief Return a string describing a getaddrinfo() or getnameinfo() error code.
 *
 * @param errcode Error code returned by getaddrinfo() or getnameinfo().
 * @return Pointer to a description string.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gai_strerror.html
 */
const char *gai_strerror(int errcode);

/**
 * @brief Translate a host name and service to a list of socket addresses.
 *
 * @param node    Host name or numeric address string, or NULL.
 * @param service Service name or numeric port string, or NULL.
 * @param hints   Criteria for address selection, or NULL for defaults.
 * @param res     Output: linked list of matching addresses.
 * @return 0 on success, or a non-zero @c EAI_* error code.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getaddrinfo.html
 */
int getaddrinfo(const char *restrict node, const char *restrict service,
		const struct addrinfo *restrict hints, struct addrinfo **restrict res);

/**
 * @brief Get the next entry from the host database.
 *
 * @return Pointer to a static @c hostent on success, or NULL at end of file or on error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/gethostent.html
 */
struct hostent *gethostent(void);

/**
 * @brief Translate a socket address to a host name and service name.
 *
 * @param sa       Socket address to look up.
 * @param salen    Size of @p sa.
 * @param host     Output buffer for the host name, or NULL.
 * @param hostlen  Size of @p host.
 * @param serv     Output buffer for the service name, or NULL.
 * @param servlen  Size of @p serv.
 * @param flags    @c NI_* flags controlling the conversion.
 * @return 0 on success, or a non-zero @c EAI_* error code.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnameinfo.html
 */
int getnameinfo(const struct sockaddr *restrict sa, socklen_t salen, char *restrict host,
		socklen_t hostlen, char *restrict serv, socklen_t servlen, int flags);

/**
 * @brief Look up a network by address.
 *
 * @param net  Network number in host byte order.
 * @param type Address type.
 * @return Pointer to a static @c netent on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetbyaddr.html
 */
struct netent *getnetbyaddr(uint32_t net, int type);

/**
 * @brief Look up a network by name.
 *
 * @param name Network name.
 * @return Pointer to a static @c netent on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetbyname.html
 */
struct netent *getnetbyname(const char *name);

/**
 * @brief Get the next entry from the network database.
 *
 * @return Pointer to a static @c netent on success, or NULL at end of file or on error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getnetent.html
 */
struct netent *getnetent(void);

/**
 * @brief Look up a protocol by name.
 *
 * @param name Protocol name.
 * @return Pointer to a static @c protoent on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobyname.html
 */
struct protoent *getprotobyname(const char *name);

/**
 * @brief Look up a protocol by number.
 *
 * @param proto Protocol number.
 * @return Pointer to a static @c protoent on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotobynumber.html
 */
struct protoent *getprotobynumber(int proto);

/**
 * @brief Get the next entry from the protocol database.
 *
 * @return Pointer to a static @c protoent on success, or NULL at end of file or on error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getprotoent.html
 */
struct protoent *getprotoent(void);

/**
 * @brief Look up a service by name and protocol.
 *
 * @param name  Service name.
 * @param proto Protocol name, or NULL for any protocol.
 * @return Pointer to a static @c servent on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservbyname.html
 */
struct servent *getservbyname(const char *name, const char *proto);

/**
 * @brief Look up a service by port number and protocol.
 *
 * @param port  Port number in network byte order.
 * @param proto Protocol name, or NULL for any protocol.
 * @return Pointer to a static @c servent on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservbyport.html
 */
struct servent *getservbyport(int port, const char *proto);

/**
 * @brief Get the next entry from the services database.
 *
 * @return Pointer to a static @c servent on success, or NULL at end of file or on error.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/getservent.html
 */
struct servent *getservent(void);

/**
 * @brief Open the host database.
 *
 * @param stayopen Non-zero to keep the database open between queries.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sethostent.html
 */
void sethostent(int stayopen);

/**
 * @brief Open the network database.
 *
 * @param stayopen Non-zero to keep the database open between queries.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setnetent.html
 */
void setnetent(int stayopen);

/**
 * @brief Open the protocol database.
 *
 * @param stayopen Non-zero to keep the database open between queries.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setprotoent.html
 */
void setprotoent(int stayopen);

/**
 * @brief Open the services database.
 *
 * @param stayopen Non-zero to keep the database open between queries.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/setservent.html
 */
void setservent(int stayopen);

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_NETDB_H_ */

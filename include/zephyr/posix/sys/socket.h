/*
 * Copyright (c) 2019 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX socket API (<sys/socket.h>)
 *
 * Provides the BSD socket interface: creating sockets, binding, connecting,
 * sending, receiving, and socket options.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_socket.h.html">
 *      POSIX.1-2017 &lt;sys/socket.h&gt;</a>
 *
 * @ingroup posix_option_group_networking
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYS_SOCKET_H_
#define ZEPHYR_INCLUDE_POSIX_SYS_SOCKET_H_

#include <sys/types.h>
#include <zephyr/net/socket.h>

/** @brief Shut down the read half of the connection.  @ingroup posix_option_group_networking*/
#define SHUT_RD   ZSOCK_SHUT_RD
/** @brief Shut down the write half of the connection.  @ingroup posix_option_group_networking*/
#define SHUT_WR   ZSOCK_SHUT_WR
/** @brief Shut down both halves of the connection.  @ingroup posix_option_group_networking*/
#define SHUT_RDWR ZSOCK_SHUT_RDWR

/** @brief Peek at incoming data without removing it from the queue.  @ingroup posix_option_group_networking*/
#define MSG_PEEK     ZSOCK_MSG_PEEK
/** @brief Return the real length of the datagram even if it was truncated.  @ingroup posix_option_group_networking*/
#define MSG_TRUNC    ZSOCK_MSG_TRUNC
/** @brief Enable non-blocking operation for this call only.  @ingroup posix_option_group_networking*/
#define MSG_DONTWAIT ZSOCK_MSG_DONTWAIT
/** @brief Block until all requested data has been received.  @ingroup posix_option_group_networking*/
#define MSG_WAITALL  ZSOCK_MSG_WAITALL

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_SOCKLEN_T_DECLARED) && !defined(__socklen_t_defined)
/** @brief Type for socket address length values.  @ingroup posix_option_group_networking*/
typedef uint32_t socklen_t;
#define _SOCKLEN_T_DECLARED
#define __socklen_t_defined
#endif

/** @brief Socket linger option structure. */
struct linger {
	int l_onoff;  /**< Non-zero to enable linger. */
	int l_linger; /**< Linger timeout in seconds. */
};

/**
 * @brief Accept a new connection on a listening socket.
 * @ingroup posix_option_group_networking
 * @param sock    Listening socket file descriptor.
 * @param addr    Output: address of the connecting peer, or NULL.
 * @param addrlen Input: size of @p addr; output: actual address size.
 * @return New socket file descriptor on success, or -1 on failure.
 */
int accept(int sock, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Assign a local address to a socket.
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param addr    Local address to bind.
 * @param addrlen Size of @p addr in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 */
int bind(int sock, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Initiate a connection on a socket.
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param addr    Remote address to connect to.
 * @param addrlen Size of @p addr in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 */
int connect(int sock, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief Get the address of the peer connected to a socket.
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param addr    Output: peer address.
 * @param addrlen Input: size of @p addr; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 */
int getpeername(int sock, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Get the local address bound to a socket.
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param addr    Output: local address.
 * @param addrlen Input: size of @p addr; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 */
int getsockname(int sock, struct sockaddr *addr, socklen_t *addrlen);

/**
 * @brief Get socket options.
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param level   Protocol level (SOL_SOCKET, IPPROTO_TCP, etc.).
 * @param optname Option name (SO_REUSEADDR, SO_KEEPALIVE, etc.).
 * @param optval  Output: option value.
 * @param optlen  Input: size of @p optval; output: actual size.
 * @return 0 on success, or -1 with errno set on failure.
 */
int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);

/**
 * @brief Mark a socket as passive (ready to accept connections).
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param backlog Maximum number of pending connections to queue.
 * @return 0 on success, or -1 with errno set on failure.
 */
int listen(int sock, int backlog);

/**
 * @brief Receive data from a connected socket.
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param buf     Buffer to receive data into.
 * @param max_len Maximum number of bytes to receive.
 * @param flags   MSG_* flags (MSG_PEEK, MSG_DONTWAIT, etc.).
 * @return Number of bytes received, 0 on connection closed, or -1 on failure.
 */
ssize_t recv(int sock, void *buf, size_t max_len, int flags);

/**
 * @brief Receive data and the sender's address from a socket.
 * @ingroup posix_option_group_networking
 * @param sock     Socket file descriptor.
 * @param buf      Buffer to receive data into.
 * @param max_len  Maximum number of bytes to receive.
 * @param flags    MSG_* flags.
 * @param src_addr Output: sender's address, or NULL.
 * @param addrlen  Input: size of @p src_addr; output: actual size.
 * @return Number of bytes received on success, or -1 on failure.
 */
ssize_t recvfrom(int sock, void *buf, size_t max_len, int flags, struct sockaddr *src_addr,
		 socklen_t *addrlen);

/**
 * @brief Receive a message (with scatter-gather I/O and ancillary data).
 * @ingroup posix_option_group_networking
 * @param sock Socket file descriptor.
 * @param msg  Message header specifying I/O vectors and address buffer.
 * @param flags MSG_* flags.
 * @return Number of bytes received on success, or -1 on failure.
 */
ssize_t recvmsg(int sock, struct msghdr *msg, int flags);

/**
 * @brief Send data on a connected socket.
 * @ingroup posix_option_group_networking
 * @param sock Socket file descriptor.
 * @param buf  Data to send.
 * @param len  Number of bytes to send.
 * @param flags MSG_* flags (MSG_DONTWAIT, MSG_NOSIGNAL, etc.).
 * @return Number of bytes sent on success, or -1 on failure.
 */
ssize_t send(int sock, const void *buf, size_t len, int flags);

/**
 * @brief Send a message (with scatter-gather I/O and ancillary data).
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param message Message header specifying I/O vectors and destination.
 * @param flags   MSG_* flags.
 * @return Number of bytes sent on success, or -1 on failure.
 */
ssize_t sendmsg(int sock, const struct msghdr *message, int flags);

/**
 * @brief Send data to a specific destination address.
 * @ingroup posix_option_group_networking
 * @param sock      Socket file descriptor.
 * @param buf       Data to send.
 * @param len       Number of bytes to send.
 * @param flags     MSG_* flags.
 * @param dest_addr Destination address.
 * @param addrlen   Size of @p dest_addr in bytes.
 * @return Number of bytes sent on success, or -1 on failure.
 */
ssize_t sendto(int sock, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr,
	       socklen_t addrlen);

/**
 * @brief Set socket options.
 * @ingroup posix_option_group_networking
 * @param sock    Socket file descriptor.
 * @param level   Protocol level (SOL_SOCKET, IPPROTO_TCP, etc.).
 * @param optname Option name.
 * @param optval  Pointer to the new option value.
 * @param optlen  Size of @p optval in bytes.
 * @return 0 on success, or -1 with errno set on failure.
 */
int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);

/**
 * @brief Shut down part or all of a full-duplex connection.
 * @ingroup posix_option_group_networking
 * @param sock Socket file descriptor.
 * @param how  SHUT_RD, SHUT_WR, or SHUT_RDWR.
 * @return 0 on success, or -1 with errno set on failure.
 */
int shutdown(int sock, int how);

/**
 * @brief Determine whether a socket is at the out-of-band mark.
 * @ingroup posix_option_group_networking
 * @param s Socket file descriptor.
 * @return 1 if at the mark, 0 if not, or -1 on failure.
 */
int sockatmark(int s);

/**
 * @brief Create a new socket.
 * @ingroup posix_option_group_networking
 * @param family Protocol family (AF_INET, AF_INET6, AF_UNIX, etc.).
 * @param type   Socket type (SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, etc.).
 * @param proto  Protocol number (IPPROTO_TCP, IPPROTO_UDP, 0 for default).
 * @return New socket file descriptor on success, or -1 on failure.
 */
int socket(int family, int type, int proto);

/**
 * @brief Create a pair of connected sockets.
 * @ingroup posix_option_group_networking
 * @param family Protocol family (typically AF_UNIX).
 * @param type   Socket type.
 * @param proto  Protocol.
 * @param sv     Output: two-element array receiving the socket descriptors.
 * @return 0 on success, or -1 with errno set on failure.
 */
int socketpair(int family, int type, int proto, int sv[2]);


#ifdef __cplusplus
}
#endif

#endif	/* ZEPHYR_INCLUDE_POSIX_SYS_SOCKET_H_ */

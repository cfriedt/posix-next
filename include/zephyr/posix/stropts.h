/*
 * Copyright (c) 2024 Abhinav Srivastava
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief XSI STREAMS interface (<stropts.h>)
 *
 * Provides the STREAMS I/O control interface (a legacy XSI extension).
 * STREAMS is rarely implemented in its entirety; Zephyr provides minimal
 * support for compatibility.
 *
 * @note STREAMS is an optional XSI extension to POSIX.  Portable applications
 *       should avoid relying on it; use BSD sockets instead.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/stropts.h.html">
 *      POSIX.1-2017 &lt;stropts.h&gt;</a>
 *
 * @defgroup posix_stropts XSI STREAMS
 * @ingroup posix_option_group_xsi_streams
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_STROPTS_H_
#define ZEPHYR_INCLUDE_POSIX_STROPTS_H_

/** @brief Flag: message carries high-priority data. */
#define RS_HIPRI BIT(0)

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Buffer descriptor for STREAMS getmsg() / putmsg() control and data parts. */
struct strbuf {
	int maxlen;  /**< Maximum buffer length. */
	int len;     /**< Length of data, or -1 to indicate no data/control part. */
	char *buf;   /**< Pointer to data buffer. */
};

/**
 * @brief Send a STREAMS message downstream.
 * @param fildes  STREAMS file descriptor.
 * @param ctlptr  Control part of the message, or NULL.
 * @param dataptr Data part of the message, or NULL.
 * @param flags   0 or RS_HIPRI.
 * @return 0 on success, or -1 with errno set on failure.
 */
int putmsg(int fildes, const struct strbuf *ctlptr, const struct strbuf *dataptr, int flags);

/**
 * @brief Detach a STREAMS-based file descriptor from a mount point.
 * @param path Pathname previously used with fattach().
 * @return 0 on success, or -1 with errno set on failure.
 */
int fdetach(const char *path);

/**
 * @brief Attach a STREAMS file descriptor to a pathname in the filesystem.
 * @param fildes STREAMS file descriptor to attach.
 * @param path   Existing pathname to attach the STREAMS fd to.
 * @return 0 on success, or -1 with errno set on failure.
 */
int fattach(int fildes, const char *path);

/**
 * @brief Receive the next message from a STREAMS file descriptor.
 * @param fildes  STREAMS file descriptor.
 * @param ctlptr  Output: control part buffer, or NULL.
 * @param dataptr Output: data part buffer, or NULL.
 * @param flagsp  Input/output: 0 or RS_HIPRI to request high-priority messages only.
 * @return 0 if complete message received, 1 if more to follow, -1 on failure.
 */
int getmsg(int fildes, struct strbuf *ctlptr, struct strbuf *dataptr, int *flagsp);

/**
 * @brief Receive a priority-banded message from a STREAMS file descriptor.
 * @param fildes  STREAMS file descriptor.
 * @param ctlptr  Output: control part buffer, or NULL.
 * @param dataptr Output: data part buffer, or NULL.
 * @param bandp   Input/output: priority band.
 * @param flagsp  Input/output: message flags.
 * @return 0 if complete message received, 1 if more to follow, -1 on failure.
 */
int getpmsg(int fildes, struct strbuf *ctlptr, struct strbuf *dataptr, int *bandp, int *flagsp);

/**
 * @brief Test whether a file descriptor refers to a STREAMS file.
 * @param fildes File descriptor to test.
 * @return 1 if @p fildes is a STREAMS file, 0 if not, -1 with errno set on error.
 */
int isastream(int fildes);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_STROPTS_H_ */

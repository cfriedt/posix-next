/*
 * Copyright (c) 2024, Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief POSIX system logging (<syslog.h>)
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/syslog.h.html">
 *      POSIX.1-2017 &lt;syslog.h&gt;</a>
 *
 * @defgroup posix_syslog POSIX System Logging
 * @ingroup posix_option_group_threads_base
 * @{
 */

#ifndef ZEPHYR_INCLUDE_POSIX_SYSLOG_H_
#define ZEPHYR_INCLUDE_POSIX_SYSLOG_H_

#include <stdarg.h>

/** @name openlog() option flags */
/** @{ */
/** @brief Include the process ID in each log message. */
#define LOG_PID    1
/** @brief Log to the system console if the logger is unavailable. */
#define LOG_CONS   2
/** @brief Open the connection to the logger immediately. */
#define LOG_NDELAY 4
/** @brief Delay the connection until the first message is sent. */
#define LOG_ODELAY 8
/** @brief Do not wait for child processes created by logging. */
#define LOG_NOWAIT 16
/** @brief Also write messages to stderr. */
#define LOG_PERROR 32
/** @} */

/** @name openlog() facility codes */
/** @{ */
#define LOG_KERN   0  /**< Kernel messages. */
#define LOG_USER   1  /**< Generic user-level messages. */
#define LOG_MAIL   2  /**< Mail system messages. */
#define LOG_NEWS   3  /**< News subsystem messages. */
#define LOG_UUCP   4  /**< UUCP subsystem messages. */
#define LOG_DAEMON 5  /**< System daemon messages. */
#define LOG_AUTH   6  /**< Security/authentication messages. */
#define LOG_CRON   7  /**< Clock daemon messages. */
#define LOG_LPR    8  /**< Printer subsystem messages. */
#define LOG_LOCAL0 9  /**< Reserved for local use (facility 0). */
#define LOG_LOCAL1 10 /**< Reserved for local use (facility 1). */
#define LOG_LOCAL2 11 /**< Reserved for local use (facility 2). */
#define LOG_LOCAL3 12 /**< Reserved for local use (facility 3). */
#define LOG_LOCAL4 13 /**< Reserved for local use (facility 4). */
#define LOG_LOCAL5 14 /**< Reserved for local use (facility 5). */
#define LOG_LOCAL6 15 /**< Reserved for local use (facility 6). */
#define LOG_LOCAL7 16 /**< Reserved for local use (facility 7). */
/** @} */

/** @name syslog() priority codes */
/** @{ */
#define LOG_EMERG   0 /**< System is unusable. */
#define LOG_ALERT   1 /**< Action must be taken immediately. */
#define LOG_CRIT    2 /**< Critical conditions. */
#define LOG_ERR     3 /**< Error conditions. */
#define LOG_WARNING 4 /**< Warning conditions. */
#define LOG_NOTICE  5 /**< Normal but significant condition. */
#define LOG_INFO    6 /**< Informational message. */
#define LOG_DEBUG   7 /**< Debug-level message. */
/** @} */

/**
 * @brief Generate a log mask for the given priority.
 * @param mask Priority value (LOG_EMERG .. LOG_DEBUG).
 * @return Bit mask with bits set for all priorities up to and including @p mask.
 */
#define LOG_MASK(mask) ((mask) & BIT_MASK(LOG_DEBUG + 1))

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Close the connection to the system logger.
 */
void closelog(void);

/**
 * @brief Open a connection to the system logger.
 * @param ident    String prepended to each log message; typically the program name.
 * @param logopt   OR of LOG_* option flags (LOG_PID, LOG_CONS, etc.).
 * @param facility Facility code (LOG_USER, LOG_DAEMON, etc.).
 */
void openlog(const char *ident, int logopt, int facility);

/**
 * @brief Set the log priority mask.
 * @param maskpri New priority mask (generated with LOG_MASK()).
 * @return The previous log priority mask.
 */
int setlogmask(int maskpri);

/**
 * @brief Write a message to the system logger.
 * @param priority Priority code (LOG_EMERG .. LOG_DEBUG).
 * @param message  printf()-style format string.
 * @param ...      Format arguments.
 */
void syslog(int priority, const char *message, ...);

/**
 * @brief Write a message to the system logger (va_list form).
 * @param priority Priority code (LOG_EMERG .. LOG_DEBUG).
 * @param format   printf()-style format string.
 * @param ap       Argument list.
 */
void vsyslog(int priority, const char *format, va_list ap);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_INCLUDE_POSIX_SYSLOG_H_ */

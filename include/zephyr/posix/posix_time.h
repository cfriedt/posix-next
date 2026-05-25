/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<time.h\>: POSIX time types, clocks, and timers
 *
 * Provides the clock identifiers, POSIX timer API, thread-safe time
 * conversion helpers, and locale-aware time formatting that extend the
 * C standard @c <time.h> interface.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/time.h.html">
 *      POSIX.1-2017 &lt;time.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_ZEPHYR_POSIX_POSIX_TIME_H_
#define ZEPHYR_INCLUDE_ZEPHYR_POSIX_POSIX_TIME_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <stddef.h>

#include <zephyr/sys/clock.h>
#include <zephyr/toolchain.h>

#ifdef __cplusplus
extern "C" {
#endif

/* clock_t must be defined in the libc time.h */
/* size_t must be defined in the libc stddef.h */
/* time_t must be defined in the libc time.h */

#if !defined(_CLOCKID_T_DECLARED) && !defined(__clockid_t_defined)
/** @brief Identifies a clock (e.g. CLOCK_REALTIME, CLOCK_MONOTONIC). */
typedef unsigned long clockid_t;
#define _CLOCKID_T_DECLARED
#define __clockid_t_defined
#endif

#if !defined(_TIMER_T_DECLARED) && !defined(__timer_t_defined)
/** @brief Opaque handle for a POSIX interval timer created with timer_create(). */
typedef unsigned long timer_t;
#define _TIMER_T_DECLARED
#define __timer_t_defined
#endif

#if !defined(_LOCALE_T_DECLARED) && !defined(__locale_t_defined)
#ifdef CONFIG_NEWLIB_LIBC
struct __locale_t;
/** @brief Opaque locale object. */
typedef struct __locale_t *locale_t;
#else
/** @brief Opaque locale object. */
typedef void *locale_t;
#endif
#define _LOCALE_T_DECLARED
#define __locale_t_defined
#endif

#if !defined(_PID_T_DECLARED) && !defined(__pid_t_defined)
/** @brief Process ID type. */
typedef int pid_t;
#define _PID_T_DECLARED
#define __pid_t_defined
#endif

struct sigevent;

/* struct tm must be defined in the libc time.h */

#if __STDC_VERSION__ >= 201112L
/* struct timespec must be defined in the libc time.h */
#else
#if !defined(_TIMESPEC_DECLARED) && !defined(__timespec_defined)
/**
 * @brief Time value with nanosecond resolution.
 * @see clock_gettime(), nanosleep()
 */
struct timespec {
	time_t tv_sec; /**< Seconds. */
	long tv_nsec;  /**< Nanoseconds [0, 999999999]. */
};
#define _TIMESPEC_DECLARED
#define __timespec_defined
#endif
#endif

#if !defined(_ITIMERSPEC_DECLARED) && !defined(__itimerspec_defined)
/** @brief Interval timer specification used with timer_settime() and timer_gettime(). */
struct itimerspec {
	struct timespec it_interval; /**< Timer interval (period); zero for a one-shot timer. */
	struct timespec it_value;    /**< Initial expiration time; zero to disarm. */
};
#define _ITIMERSPEC_DECLARED
#define __itimerspec_defined
#endif

/* NULL must be defined in the libc stddef.h */

/** @brief Clock measuring real (wall-clock) time.  @ingroup posix_option_group_timers*/
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME ((clockid_t)SYS_CLOCK_REALTIME)
#endif

/** @brief Number of clock ticks per second as seen by clock().  @ingroup posix_option_group_timers*/
#ifndef CLOCKS_PER_SEC
#if defined(_XOPEN_SOURCE)
#define CLOCKS_PER_SEC 1000000
#else
#define CLOCKS_PER_SEC CONFIG_SYS_CLOCK_TICKS_PER_SEC
#endif
#endif

#if defined(_POSIX_CPUTIME) || defined(__DOXYGEN__)
/** @brief CPU-time clock for the calling process.  @ingroup posix_option_cputime*/
#ifndef CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_PROCESS_CPUTIME_ID ((clockid_t)2)
#endif
#endif

#if defined(_POSIX_THREAD_CPUTIME) || defined(__DOXYGEN__)
/** @brief CPU-time clock for the calling thread.  @ingroup posix_option_thread_cputime*/
#ifndef CLOCK_THREAD_CPUTIME_ID
#define CLOCK_THREAD_CPUTIME_ID ((clockid_t)3)
#endif
#endif

#if defined(_POSIX_MONOTONIC_CLOCK) || defined(__DOXYGEN__)
/** @brief Monotonically increasing clock that cannot be set.  @ingroup posix_option_monotonic_clock*/
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC ((clockid_t)SYS_CLOCK_MONOTONIC)
#endif
#endif

/** @brief Flag for clock_nanosleep() and timer_settime(): interpret time as absolute.  @ingroup posix_option_group_timers*/
#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME SYS_TIMER_ABSTIME
#endif

/* asctime() must be declared in the libc time.h */

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) || defined(__DOXYGEN__)
/**
 * @brief Convert broken-down time to a string (thread-safe version of asctime()).
 * @ingroup posix_option_group_c_lang_support_r
 * @param tm  Broken-down time.
 * @param buf Caller-supplied buffer of at least 26 bytes.
 * @return @p buf on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/asctime_r.html
 */
char *asctime_r(const struct tm *ZRESTRICT tm, char *ZRESTRICT buf);
#endif

/* clock() must be declared in the libc time.h */

#if defined(_POSIX_CPUTIME) || defined(__DOXYGEN__)
/**
 * @brief Get the CPU-time clock ID for a process.
 * @ingroup posix_option_cputime
 * @param pid      Process ID (0 = calling process).
 * @param clock_id Output: clock ID.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_getcpuclockid.html
 */
int clock_getcpuclockid(pid_t pid, clockid_t *clock_id);
#endif

#if defined(_POSIX_TIMERS) || defined(__DOXYGEN__)
/**
 * @brief Get the resolution of a clock.
 * @ingroup posix_option_group_timers
 * @param clock_id Clock to query.
 * @param ts       Output: resolution (smallest representable interval).
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_getres.html
 */
int clock_getres(clockid_t clock_id, struct timespec *ts);

/**
 * @brief Get the current time of a clock.
 * @ingroup posix_option_group_timers
 * @param clock_id Clock to read.
 * @param ts       Output: current time.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_gettime.html
 */
int clock_gettime(clockid_t clock_id, struct timespec *ts);
#endif

#if defined(_POSIX_CLOCK_SELECTION) || defined(__DOXYGEN__)
/**
 * @brief High-resolution sleep against a specified clock.
 * @ingroup posix_option_group_clock_selection
 * @param clock_id Clock to measure the sleep against.
 * @param flags    0 for a relative sleep, @c TIMER_ABSTIME for an absolute wakeup time.
 * @param rqtp     Requested sleep duration or absolute wakeup time.
 * @param rmtp     Output: remaining time if interrupted (only for relative sleeps), or NULL.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_nanosleep.html
 */
int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp,
		    struct timespec *rmtp);
#endif

#if defined(_POSIX_TIMERS) || defined(__DOXYGEN__)
/**
 * @brief Set the time of a clock.
 * @ingroup posix_option_group_timers
 * @param clock_id Clock to set.
 * @param ts       New time value.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/clock_settime.html
 */
int clock_settime(clockid_t clock_id, const struct timespec *ts);
#endif

/* ctime() must be declared in the libc time.h */

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) || defined(__DOXYGEN__)
/**
 * @brief Convert a time_t to a string (thread-safe version of ctime()).
 * @ingroup posix_option_group_c_lang_support_r
 * @param clock Pointer to a time_t value.
 * @param buf   Caller-supplied buffer of at least 26 bytes.
 * @return @p buf on success, or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/ctime_r.html
 */
char *ctime_r(const time_t *clock, char *buf);
#endif

/* difftime() must be declared in the libc time.h */

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Convert a date-time string to broken-down time (XSI extension).
 * @ingroup posix_option_group_xsi_single_process
 * @param string Date-time string; the format is determined by the DATEMSK environment variable.
 * @return Pointer to a broken-down time, or NULL on failure.
 */
struct tm *getdate(const char *string);
#endif

/* gmtime() must be declared in the libc time.h */
#if __STDC_VERSION__ >= 202311L
/* gmtime_r() must be declared in the libc time.h */
#else
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) || defined(__DOXYGEN__)
/**
 * @brief Convert a time_t to UTC broken-down time (thread-safe version of gmtime()).
 * @ingroup posix_option_group_c_lang_support_r
 * @param timer  Pointer to the time_t value.
 * @param result Caller-supplied storage for the result.
 * @return @p result on success, or NULL on failure.
 */
struct tm *gmtime_r(const time_t *ZRESTRICT timer, struct tm *ZRESTRICT result);
#endif
#endif

/* localtime() must be declared in the libc time.h */
#if __STDC_VERSION__ >= 202311L
/* localtime_r() must be declared in the libc time.h */
#else
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) || defined(__DOXYGEN__)
/**
 * @brief Convert a time_t to local broken-down time (thread-safe version of localtime()).
 * @ingroup posix_option_group_c_lang_support_r
 * @param timer  Pointer to the time_t value.
 * @param result Caller-supplied storage for the result.
 * @return @p result on success, or NULL on failure.
 */
struct tm *localtime_r(const time_t *ZRESTRICT timer, struct tm *ZRESTRICT result);
#endif
#endif

/* mktime() must be declared in the libc time.h */

#if defined(_POSIX_TIMERS) || defined(__DOXYGEN__)
/**
 * @brief Sleep for a specified number of nanoseconds (high-resolution sleep).
 * @ingroup posix_option_group_timers
 * @param rqtp Requested sleep duration.
 * @param rmtp Output: remaining time if the call was interrupted, or NULL.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/nanosleep.html
 */
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
#endif

/* strftime() must be declared in the libc time.h */

/**
 * @brief Format broken-down time into a string using the specified locale.
 * @ingroup posix_option_group_c_lib_ext
 * @param s       Output buffer.
 * @param maxsize Maximum number of bytes to write including the NUL terminator.
 * @param format  Format string (same syntax as strftime()).
 * @param timeptr Broken-down time to format.
 * @param locale  Locale to use for formatting.
 * @return Number of bytes written (excluding NUL) on success, or 0 if @p maxsize was exceeded.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strftime_l.html
 */
size_t strftime_l(char *ZRESTRICT s, size_t maxsize, const char *ZRESTRICT format,
		  const struct tm *ZRESTRICT timeptr, locale_t locale);

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Parse a date/time string according to a format (XSI extension).
 * @ingroup posix_option_group_xsi_single_process
 * @param s      Input string to parse.
 * @param format Format string describing the date/time fields to parse.
 * @param tm     Output: filled-in broken-down time structure.
 * @return Pointer to the first character in @p s not consumed by parsing,
 *         or NULL on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/strptime.html
 */
char *strptime(const char *ZRESTRICT s, const char *ZRESTRICT format, struct tm *ZRESTRICT tm);
#endif

/* time() must be declared in the libc time.h */

#if defined(_POSIX_TIMERS) || defined(__DOXYGEN__)
/**
 * @brief Create a per-process timer.
 * @ingroup posix_option_group_timers
 * @param clockId Clock to base the timer on.
 * @param evp     Notification specification, or NULL for SIGALRM delivery.
 * @param timerid Output: new timer ID.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/timer_create.html
 */
int timer_create(clockid_t clockId, struct sigevent *ZRESTRICT evp, timer_t *ZRESTRICT timerid);

/**
 * @brief Delete a per-process timer.
 * @ingroup posix_option_group_timers
 * @param timerid Timer to delete.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/timer_delete.html
 */
int timer_delete(timer_t timerid);

/**
 * @brief Get the number of timer overruns since the last timer expiration notification.
 * @ingroup posix_option_group_timers
 * @param timerid Timer to query.
 * @return Overrun count on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/timer_getoverrun.html
 */
int timer_getoverrun(timer_t timerid);

/**
 * @brief Get the time remaining until the next timer expiration.
 * @ingroup posix_option_group_timers
 * @param timerid Timer to query.
 * @param its     Output: current timer value and interval.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/timer_gettime.html
 */
int timer_gettime(timer_t timerid, struct itimerspec *its);

/**
 * @brief Arm or disarm a per-process timer.
 * @ingroup posix_option_group_timers
 * @param timerid Timer to set.
 * @param flags   0 for a relative time, @c TIMER_ABSTIME for an absolute time.
 * @param value   New expiration time and interval; set it_value to zero to disarm.
 * @param ovalue  Output: previous timer value, or NULL.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/timer_settime.html
 */
int timer_settime(timer_t timerid, int flags, const struct itimerspec *value,
		  struct itimerspec *ovalue);
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Non-zero if Daylight Saving Time is in effect. */
/** @ingroup posix_option_group_xsi_single_process */
extern int daylight;
/** @brief Offset in seconds from UTC for the current timezone. */
/** @ingroup posix_option_group_xsi_single_process */
extern long timezone;
#endif

/** @brief Timezone abbreviations; tzname[0] = std, tzname[1] = dst */
/** @ingroup posix_option_group_c_lang_support */
extern char *tzname[];



#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_ZEPHYR_POSIX_POSIX_TIME_H_ */

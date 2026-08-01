/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<limits.h\>: POSIX symbolic constants for limits
 *
 * Defines minimum, maximum, and runtime-invariant POSIX limits as required
 * by POSIX.1-2017.  These constants complement the C standard @c <limits.h>
 * header and the runtime sysconf()/pathconf() interfaces.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/limits.h.html">
 *      POSIX.1-2017 &lt;limits.h&gt;</a>
 */

#ifndef ZEPHYR_INCLUDE_ZEPHYR_POSIX_POSIX_LIMITS_H_
#define ZEPHYR_INCLUDE_ZEPHYR_POSIX_POSIX_LIMITS_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

/*
 * clang-format and checkpatch disagree on formatting here, so rely on checkpatch and disable
 * clang-format since checkpatch cannot be selectively disabled.
 */

/* clang-format off */

#undef _POSIX_CLOCKRES_MIN
/** @brief Maximum nanoseconds between clock ticks (20 ms). */
#define _POSIX_CLOCKRES_MIN (20000000L)
#undef _POSIX_AIO_LISTIO_MAX
/** @brief Minimum number of I/O operations in a single list I/O call. */
#define _POSIX_AIO_LISTIO_MAX               (2)
#undef _POSIX_AIO_MAX
#define _POSIX_AIO_MAX                      (1)
#undef _POSIX_ARG_MAX
#define _POSIX_ARG_MAX                      (4096)
#undef _POSIX_CHILD_MAX
#define _POSIX_CHILD_MAX                    (25)
#undef _POSIX_DELAYTIMER_MAX
#define _POSIX_DELAYTIMER_MAX               (32)
#undef _POSIX_HOST_NAME_MAX
#define _POSIX_HOST_NAME_MAX                (255)
#undef _POSIX_LINK_MAX
#define _POSIX_LINK_MAX                     (8)
#undef _POSIX_LOGIN_NAME_MAX
#define _POSIX_LOGIN_NAME_MAX               (9)
#undef _POSIX_MAX_CANON
#define _POSIX_MAX_CANON                    (255)
#undef _POSIX_MAX_INPUT
#define _POSIX_MAX_INPUT                    (255)
#undef _POSIX_MQ_OPEN_MAX
#define _POSIX_MQ_OPEN_MAX                  (8)
#undef _POSIX_MQ_PRIO_MAX
#define _POSIX_MQ_PRIO_MAX                  (32)
#undef _POSIX_NAME_MAX
#define _POSIX_NAME_MAX                     (14)
#undef _POSIX_NGROUPS_MAX
#define _POSIX_NGROUPS_MAX                  (8)
#undef _POSIX_OPEN_MAX
#define _POSIX_OPEN_MAX                     (20)
#undef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX                     (256)
#undef _POSIX_PIPE_BUF
#define _POSIX_PIPE_BUF                     (512)
#undef _POSIX_RE_DUP_MAX
#define _POSIX_RE_DUP_MAX                   (255)
#undef _POSIX_RTSIG_MAX
#define _POSIX_RTSIG_MAX                    (8)
#undef _POSIX_SEM_NSEMS_MAX
#define _POSIX_SEM_NSEMS_MAX                (256)
#undef _POSIX_SEM_VALUE_MAX
#define _POSIX_SEM_VALUE_MAX                (32767)
#undef _POSIX_SIGQUEUE_MAX
#define _POSIX_SIGQUEUE_MAX                 (32)
#undef _POSIX_SSIZE_MAX
#define _POSIX_SSIZE_MAX                    (32767)
#undef _POSIX_SS_REPL_MAX
#define _POSIX_SS_REPL_MAX                  (4)
#undef _POSIX_STREAM_MAX
#define _POSIX_STREAM_MAX                   (8)
#undef _POSIX_SYMLINK_MAX
#define _POSIX_SYMLINK_MAX                  (255)
#undef _POSIX_SYMLOOP_MAX
#define _POSIX_SYMLOOP_MAX                  (8)
#undef _POSIX_THREAD_DESTRUCTOR_ITERATIONS
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS (4)
#undef _POSIX_THREAD_KEYS_MAX
#define _POSIX_THREAD_KEYS_MAX              (128)
#undef _POSIX_THREAD_THREADS_MAX
#define _POSIX_THREAD_THREADS_MAX           (64)
#undef _POSIX_TIMER_MAX
#define _POSIX_TIMER_MAX                    (32)
#undef _POSIX_TRACE_EVENT_NAME_MAX
#define _POSIX_TRACE_EVENT_NAME_MAX         (30)
#undef _POSIX_TRACE_NAME_MAX
#define _POSIX_TRACE_NAME_MAX               (8)
#undef _POSIX_TRACE_SYS_MAX
#define _POSIX_TRACE_SYS_MAX                (8)
#undef _POSIX_TRACE_USER_EVENT_MAX
#define _POSIX_TRACE_USER_EVENT_MAX         (32)
#undef _POSIX_TTY_NAME_MAX
#define _POSIX_TTY_NAME_MAX                 (9)
#undef _POSIX_TZNAME_MAX
#define _POSIX_TZNAME_MAX                   (6)
#undef _POSIX2_BC_BASE_MAX
#define _POSIX2_BC_BASE_MAX                 (99)
#undef _POSIX2_BC_DIM_MAX
#define _POSIX2_BC_DIM_MAX                  (2048)
#undef _POSIX2_BC_SCALE_MAX
#define _POSIX2_BC_SCALE_MAX                (99)
#undef _POSIX2_BC_STRING_MAX
#define _POSIX2_BC_STRING_MAX               (1000)
#undef _POSIX2_CHARCLASS_NAME_MAX
#define _POSIX2_CHARCLASS_NAME_MAX          (14)
#undef _POSIX2_COLL_WEIGHTS_MAX
#define _POSIX2_COLL_WEIGHTS_MAX            (2)
#undef _POSIX2_EXPR_NEST_MAX
#define _POSIX2_EXPR_NEST_MAX               (32)
#undef _POSIX2_LINE_MAX
#define _POSIX2_LINE_MAX                    (2048)
#undef _XOPEN_IOV_MAX
#define _XOPEN_IOV_MAX                      (16)
#undef _XOPEN_NAME_MAX
#define _XOPEN_NAME_MAX                     (255)
#undef _XOPEN_PATH_MAX
#define _XOPEN_PATH_MAX                     (1024)


#undef NL_LANGMAX
/** @brief Maximum number of bytes in a LANG name. */
#define NL_LANGMAX (14)
#undef NL_MSGMAX
/** @brief Maximum message number. */
#define NL_MSGMAX  (32767)
#undef NL_SETMAX
/** @brief Maximum set number. */
#define NL_SETMAX  (255)
#undef NL_TEXTMAX
/** @brief Maximum number of bytes in a message string. */
#define NL_TEXTMAX (_POSIX2_LINE_MAX)
#undef NZERO
/** @brief Default process priority (nice value). */
#define NZERO      (20)
#undef AIO_LISTIO_MAX
#define AIO_LISTIO_MAX \
	COND_CODE_1(CONFIG_POSIX_ASYNCHRONOUS_IO, (CONFIG_POSIX_AIO_LISTIO_MAX), (0))
#undef AIO_MAX
#define AIO_MAX \
	COND_CODE_1(CONFIG_POSIX_ASYNCHRONOUS_IO, (CONFIG_POSIX_AIO_MAX), (0))
#undef AIO_PRIO_DELTA_MAX
#define AIO_PRIO_DELTA_MAX \
	COND_CODE_1(CONFIG_POSIX_PRIORITIZED_IO, \
		    (CONFIG_NUM_COOP_PRIORITIES + CONFIG_NUM_PREEMPT_PRIORITIES - 1), (0))
#undef ARG_MAX
#define ARG_MAX                       _POSIX_ARG_MAX
#undef ATEXIT_MAX
#define ATEXIT_MAX                    (32)
#undef DELAYTIMER_MAX
#define DELAYTIMER_MAX \
	COND_CODE_1(CONFIG_POSIX_TIMERS, (CONFIG_POSIX_DELAYTIMER_MAX), (0))
#undef HOST_NAME_MAX
#define HOST_NAME_MAX \
	COND_CODE_1(CONFIG_POSIX_NETWORKING, (CONFIG_POSIX_HOST_NAME_MAX), (0))
#undef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX                _POSIX_LOGIN_NAME_MAX
#undef MQ_OPEN_MAX
#define MQ_OPEN_MAX \
	COND_CODE_1(CONFIG_POSIX_MESSAGE_PASSING, (CONFIG_POSIX_MQ_OPEN_MAX), (0))
#undef MQ_PRIO_MAX
#define MQ_PRIO_MAX                   _POSIX_MQ_PRIO_MAX
#undef OPEN_MAX
#define OPEN_MAX                      CONFIG_POSIX_OPEN_MAX
#undef PAGE_SIZE
#define PAGE_SIZE                     CONFIG_POSIX_PAGE_SIZE
#undef PAGESIZE
#define PAGESIZE                      CONFIG_POSIX_PAGE_SIZE
#undef PATH_MAX
#define PATH_MAX                      _POSIX_PATH_MAX
#undef PTHREAD_DESTRUCTOR_ITERATIONS
#define PTHREAD_DESTRUCTOR_ITERATIONS _POSIX_THREAD_DESTRUCTOR_ITERATIONS
#undef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX \
	COND_CODE_1(CONFIG_POSIX_THREADS, (CONFIG_POSIX_THREAD_KEYS_MAX), (0))
#undef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN CONFIG_IDLE_STACK_SIZE
#undef PTHREAD_THREADS_MAX
#define PTHREAD_THREADS_MAX \
	COND_CODE_1(CONFIG_POSIX_THREADS, (CONFIG_POSIX_THREAD_THREADS_MAX), (0))
#undef RTSIG_MAX
#define RTSIG_MAX \
	COND_CODE_1(CONFIG_POSIX_REALTIME_SIGNALS, (CONFIG_POSIX_RTSIG_MAX), (0))
#undef SEM_NSEMS_MAX
#define SEM_NSEMS_MAX \
	COND_CODE_1(CONFIG_POSIX_SEMAPHORES, (CONFIG_POSIX_SEM_NSEMS_MAX), (0))
#undef SEM_VALUE_MAX
#define SEM_VALUE_MAX \
	COND_CODE_1(CONFIG_POSIX_SEMAPHORES, (CONFIG_POSIX_SEM_VALUE_MAX), (0))
#undef SIGQUEUE_MAX
#define SIGQUEUE_MAX                  _POSIX_SIGQUEUE_MAX
#undef STREAM_MAX
#define STREAM_MAX                    _POSIX_STREAM_MAX
#undef SYMLOOP_MAX
#define SYMLOOP_MAX                   _POSIX_SYMLOOP_MAX
#undef TIMER_MAX
#define TIMER_MAX \
	COND_CODE_1(CONFIG_POSIX_TIMERS, (CONFIG_POSIX_TIMER_MAX), (0))
#undef TTY_NAME_MAX
#define TTY_NAME_MAX                  _POSIX_TTY_NAME_MAX
#undef TZNAME_MAX
#define TZNAME_MAX                    _POSIX_TZNAME_MAX


#undef FILESIZEBITS
/** @brief Minimum number of bits needed to represent file size. */
#define FILESIZEBITS             (32)
#undef POSIX_ALLOC_SIZE_MIN
#define POSIX_ALLOC_SIZE_MIN     (256)
#undef POSIX_REC_INCR_XFER_SIZE
#define POSIX_REC_INCR_XFER_SIZE (1024)
#undef POSIX_REC_MAX_XFER_SIZE
#define POSIX_REC_MAX_XFER_SIZE  (32767)
#undef POSIX_REC_MIN_XFER_SIZE
#define POSIX_REC_MIN_XFER_SIZE  (1)
#undef POSIX_REC_XFER_ALIGN
#define POSIX_REC_XFER_ALIGN     (4)
#undef SYMLINK_MAX
#define SYMLINK_MAX              _POSIX_SYMLINK_MAX


/* clang-format on */


#endif

#endif /* ZEPHYR_INCLUDE_ZEPHYR_POSIX_POSIX_LIMITS_H_ */

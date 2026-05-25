/*
 * Copyright (c) 2025 The Zephyr Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief \<signal.h\>: POSIX signal types and functions
 *
 * Provides signal numbers, signal sets, signal actions, real-time signal
 * extensions, and the full set of POSIX signal-management functions.
 *
 * @see <a href="https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/signal.h.html">
 *      POSIX.1-2017 &lt;signal.h&gt;</a>
 *
 */

#ifndef ZEPHYR_INCLUDE_POSIX_POSIX_SIGNAL_H_
#define ZEPHYR_INCLUDE_POSIX_POSIX_SIGNAL_H_

#if defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__)

#include <zephyr/toolchain.h>
#include <zephyr/sys/util.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SIG_DFL must be defined by the libc signal.h */
/* SIG_ERR must be defined by the libc signal.h */

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Signal disposition: hold the signal (XSI extension, used with sigset()). */
#define SIG_HOLD ((void *)-2)
#endif

/* SIG_IGN must be defined by the libc signal.h */

#if defined(_POSIX_THREADS) || defined(__DOXYGEN__)

#if !defined(_PTHREAD_T_DECLARED) && !defined(__pthread_t_defined)
typedef unsigned int pthread_t;
#define _PTHREAD_T_DECLARED
#define __pthread_t_defined
#endif

#endif /* defined(_POSIX_THREADS) || defined(__DOXYGEN__) */

/* size_t must be defined by the libc stddef.h */
#include <stddef.h>

#if !defined(_UID_T_DECLARED) && !defined(__uid_t_defined)
typedef int uid_t;
#define _UID_T_DECLARED
#define __uid_t_defined
#endif

/* time_t must be defined by the libc time.h */
#include <time.h>

#if __STDC_VERSION__ >= 201112L
/* struct timespec must be defined in the libc time.h */
#else
#if !defined(_TIMESPEC_DECLARED) && !defined(__timespec_defined)
struct timespec {
	time_t tv_sec;
	long tv_nsec;
};
#define _TIMESPEC_DECLARED
#define __timespec_defined
#endif
#endif

/* sig_atomic_t must be defined by the libc signal.h */

#define SIGRTMIN 32
#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)
BUILD_ASSERT(CONFIG_POSIX_RTSIG_MAX >= 0);
#define SIGRTMAX (SIGRTMIN + CONFIG_POSIX_RTSIG_MAX)
#else
#define SIGRTMAX SIGRTMIN
#endif

#if !defined(_SIGSET_T_DECLARED) && !defined(__sigset_t_defined)
/** @brief Type representing a set of signals (bitmask). */
typedef struct {
	unsigned long sig[DIV_ROUND_UP(SIGRTMAX, BITS_PER_LONG)];
} sigset_t;
#define _SIGSET_T_DECLARED
#define __sigset_t_defined
#endif

#if !defined(_PID_T_DECLARED) && !defined(__pid_t_defined)
typedef long pid_t;
#define _PID_T_DECLARED
#define __pid_t_defined
#endif

#if defined(_POSIX_THREADS) || defined(__DOXYGEN__)

#if !defined(_PTHREAD_ATTR_T_DECLARED) && !defined(__pthread_attr_t_defined)
typedef struct {
	void *stack;
	unsigned int details[2];
} pthread_attr_t;
#define _PTHREAD_ATTR_T_DECLARED
#define __pthread_attr_t_defined
#endif

#endif

#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)

/* slightly out of order w.r.t. the specification */
#if !defined(_SIGVAL_DECLARED) && !defined(__sigval_defined)
/** @brief Value passed to a signal handler or retrieved via siginfo_t. */
union sigval {
	int sival_int;   /**< Integer value. */
	void *sival_ptr; /**< Pointer value. */
};
#define _SIGVAL_DECLARED
#define __sigval_defined
#endif

#if !defined(_SIGEVENT_DECLARED) && !defined(__sigevent_defined)
/** @brief Structure describing how to notify about an asynchronous event. */
struct sigevent {
#if defined(_POSIX_THREADS) || defined(__DOXYGEN__)
	pthread_attr_t *sigev_notify_attributes; /**< Thread attributes for SIGEV_THREAD. */
	void (*sigev_notify_function)(union sigval value); /**< Notification function. */
#endif
	union sigval sigev_value;  /**< Value passed to notification function or signal. */
	int sigev_notify;          /**< Notification type: SIGEV_NONE, SIGEV_SIGNAL, SIGEV_THREAD. */
	int sigev_signo;           /**< Signal number for SIGEV_SIGNAL notifications. */
};
#define _SIGEVENT_DECLARED
#define __sigevent_defined
#endif

/** @brief No notification on event completion. */
#define SIGEV_NONE   1
/** @brief Send a signal on event completion. */
#define SIGEV_SIGNAL 2
/** @brief Call a function in a new thread on event completion. */
#define SIGEV_THREAD 3

/* Signal constants are defined below */

#endif /* defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__) */

/* SIGRTMIN and SIGRTMAX defined above */

/* slightly out of order w.r.t. the specification */
#if !defined(_SIGINFO_T_DECLARED) && !defined(__siginfo_t_defined)
/** @brief Information associated with a received signal. */
typedef struct {
	void *si_addr;           /**< Address of the faulting instruction (SIGILL, SIGFPE, SIGSEGV, SIGBUS). */
#if defined(_XOPEN_STREAMS) || defined(__DOXYGEN__)
	long si_band;            /**< Band event number for SIGPOLL (XSI streams). */
#endif
	union sigval si_value;   /**< Signal value (real-time signals). */
	pid_t si_pid;            /**< Sending process ID. */
	uid_t si_uid;            /**< Real UID of the sending process. */
	int si_signo;            /**< Signal number. */
	int si_code;             /**< Signal code (reason the signal was generated). */
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
	int si_errno;            /**< errno value associated with this signal, or 0. */
#endif
	int si_status;           /**< Exit value or signal (SIGCHLD). */
} siginfo_t;
#define _SIGINFO_T_DECLARED
#define __siginfo_t_defined
#endif

#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)

#if !defined(_SIGACTION_DECLARED) && !defined(__sigaction_defined)
/** @brief Signal action structure used with sigaction(). */
struct sigaction {
	union {
		void (*sa_handler)(int sig);                          /**< Simple handler (SA_SIGINFO not set). */
		void (*sa_sigaction)(int sig, siginfo_t *info, void *context); /**< Extended handler (SA_SIGINFO set). */
	};
	sigset_t sa_mask;   /**< Signals blocked during handler execution. */
	int sa_flags;       /**< Flags modifying signal behaviour (SA_* constants). */
};
#define _SIGACTION_DECLARED
#define __sigaction_defined
#endif

/** @brief Block the signals in @p set. */
#define SIG_BLOCK   1
/** @brief Unblock the signals in @p set. */
#define SIG_UNBLOCK 2
/** @brief Set the signal mask to @p set. */
#define SIG_SETMASK 0

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Do not generate SIGCHLD when child processes stop (XSI). */
#define SA_NOCLDSTOP 0x00000001
/** @brief Invoke the handler on an alternate signal stack. */
#define SA_ONSTACK   0x00000002
#endif
/** @brief Reset the signal disposition to SIG_DFL after delivery. */
#define SA_RESETHAND 0x00000004
/** @brief Restart interrupted system calls instead of returning EINTR. */
#define SA_RESTART   0x00000008
/** @brief Invoke the sa_sigaction handler instead of sa_handler. */
#define SA_SIGINFO   0x00000010
#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/** @brief Do not create zombie processes for terminated child processes (XSI). */
#define SA_NOCLDWAIT 0x00000020
#endif
/** @brief Do not add the signal to the process signal mask during handler execution. */
#define SA_NODEFER  0x00000040
/** @brief Alternate signal stack is active (ss_flags value). */
#define SS_ONSTACK  0x00000001
/** @brief Alternate signal stack is disabled (ss_flags value). */
#define SS_DISABLE  0x00000002
/** @brief Minimum stack size for a signal handler. */
#define MINSIGSTKSZ 4096
/** @brief Default stack size for a signal handler. */
#define SIGSTKSZ    4096

#if !defined(_MCONTEXT_T_DECLARED) && !defined(__mcontext_t_defined)
/** @brief Machine-specific context saved when a signal is delivered. */
typedef struct {
	/* FIXME: there should be a much better Zephyr-specific structure that can be used here */
	unsigned long gregs[32]; /**< General-purpose registers. */
	unsigned long flags;     /**< Architecture-specific flags. */
} mcontext_t;
#define _MCONTEXT_T_DECLARED
#define __mcontext_defined
#endif

/* slightly out of order w.r.t. the specification */
#if !defined(_STACK_T_DECLARED) && !defined(__stack_t_defined)
/** @brief Alternate signal stack descriptor. */
typedef struct {
	void *ss_sp;      /**< Stack base address. */
	size_t ss_size;   /**< Stack size in bytes. */
	int ss_flags;     /**< SS_ONSTACK or SS_DISABLE. */
} stack_t;
#define _STACK_T_DECLARED
#define __stack_t_defined
#endif

#if !defined(_UCONTEXT_T_DECLARED) && !defined(__ucontext_t_defined)
/** @brief User-space context saved and restored by getcontext()/setcontext(). */
typedef struct {
	struct ucontext *uc_link;  /**< Context to resume when this one returns. */
	sigset_t uc_sigmask;       /**< Signals blocked in this context. */
	stack_t uc_stack;          /**< Stack used by this context. */
	mcontext_t uc_mcontext;    /**< Machine-specific saved state. */
} ucontext_t;
#define _UCONTEXT_T_DECLARED
#define __ucontext_defined
#endif

#endif /* defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__) */

/* Siginfo codes are defined below */

#if !defined(_SIGHANDLER_T_DECLARED) && !defined(__sighandler_t_defined)
/** @brief Function pointer type for a simple signal handler. */
typedef void (*sighandler_t)(int sig);
#define _SIGHANDLER_T_DECLARED
#define __sighandler_t_defined
#endif

/**
 * @brief Send a signal to a process or process group.
 * @ingroup posix_option_group_signals
 * @param pid  Target process ID (positive), process group (negative), or 0.
 * @param sig  Signal number, or 0 to check process existence.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/kill.html
 */
int kill(pid_t pid, int sig);

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Send a signal to a process group (XSI extension).
 * @ingroup posix_option_group_signals
 * @param pgrp Process group ID (0 = calling process's group).
 * @param sig  Signal number.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/killpg.html
 */
int killpg(pid_t pgrp, int sig);
#endif

/**
 * @brief Print a signal description with additional siginfo_t context.
 * @ingroup posix_option_group_signals
 * @param info    Signal information.
 * @param message Prefix string.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/psiginfo.html
 */
void psiginfo(const siginfo_t *info, const char *message);

/**
 * @brief Print a signal description to stderr.
 * @ingroup posix_option_group_signals
 * @param sig     Signal number.
 * @param message Prefix string.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/psignal.html
 */
void psignal(int sig, const char *message);

#if defined(_POSIX_THREADS) || defined(__DOXYGEN__)
/**
 * @brief Send a signal to a specific thread.
 * @ingroup posix_option_group_threads_base
 * @param thread Target thread.
 * @param sig    Signal number, or 0 to check thread existence.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_kill.html
 */
int pthread_kill(pthread_t thread, int sig);

/**
 * @brief Examine and change blocked signals for the calling thread.
 * @ingroup posix_option_group_threads_base
 * @param how  @c SIG_BLOCK, @c SIG_UNBLOCK, or @c SIG_SETMASK.
 * @param set  Signal set to apply, or NULL.
 * @param oset Output: previous signal mask, or NULL.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_sigmask.html
 */
int pthread_sigmask(int how, const sigset_t *ZRESTRICT set, sigset_t *ZRESTRICT oset);
#endif

#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)
TOOLCHAIN_DISABLE_WARNING(TOOLCHAIN_WARNING_SHADOW);
/**
 * @brief Examine and change a signal action.
 * @ingroup posix_option_group_signals
 * @param sig  Signal number.
 * @param act  New action, or NULL to query.
 * @param oact Output: previous action, or NULL.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaction.html
 */
int sigaction(int sig, const struct sigaction *ZRESTRICT act, struct sigaction *ZRESTRICT oact);
TOOLCHAIN_ENABLE_WARNING(TOOLCHAIN_WARNING_SHADOW);
#endif

/**
 * @brief Add a signal to a signal set.
 * @ingroup posix_option_group_signals
 * @param set Signal set.
 * @param sig Signal number to add.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaddset.html
 */
int sigaddset(sigset_t *set, int sig);

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Set or get the alternate signal stack (XSI extension).
 * @ingroup posix_option_group_signals
 * @param ss  New alternate stack descriptor, or NULL.
 * @param oss Output: previous descriptor, or NULL.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaltstack.html
 */
int sigaltstack(const stack_t *ZRESTRICT ss, stack_t *ZRESTRICT oss);
#endif

/**
 * @brief Delete a signal from a signal set.
 * @ingroup posix_option_group_signals
 * @param set Signal set.
 * @param sig Signal number to remove.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigdelset.html
 */
int sigdelset(sigset_t *set, int sig);

/**
 * @brief Initialise a signal set to the empty set.
 * @ingroup posix_option_group_signals
 * @param set Signal set to clear.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigemptyset.html
 */
int sigemptyset(sigset_t *set);

/**
 * @brief Initialise a signal set to the full set (all signals).
 * @ingroup posix_option_group_signals
 * @param set Signal set to fill.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigfillset.html
 */
int sigfillset(sigset_t *set);

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Add a signal to the calling process's signal mask (XSI, obsolescent).
 * @ingroup posix_option_group_signals
 * @param sig Signal to block.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sighold.html
 */
int sighold(int sig);

/**
 * @brief Set a signal's disposition to SIG_IGN (XSI, obsolescent).
 * @ingroup posix_option_group_signals
 * @param sig Signal to ignore.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigignore.html
 */
int sigignore(int sig);

/**
 * @brief Control whether a signal restarts or interrupts system calls (XSI, obsolescent).
 * @ingroup posix_option_group_signals
 * @param sig  Signal number.
 * @param flag Non-zero to interrupt; 0 to restart.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/siginterrupt.html
 */
int siginterrupt(int sig, int flag);
#endif

/**
 * @brief Test whether a signal is a member of a signal set.
 * @ingroup posix_option_group_signals
 * @param set Signal set to query.
 * @param sig Signal number to test.
 * @return 1 if the signal is a member, 0 if not, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigismember.html
 */
int sigismember(const sigset_t *set, int sig);

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Suspend execution until a signal is delivered (XSI, obsolescent).
 * @ingroup posix_option_group_signals
 * @param sig Signal whose blocking is temporarily removed.
 * @return Always returns -1 with @c errno set to @c EINTR.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigpause.html
 */
int sigpause(int sig);
#endif

/**
 * @brief Retrieve the set of pending signals.
 * @ingroup posix_option_group_signals
 * @param set Output: set of signals pending delivery to the calling process.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigpending.html
 */
int sigpending(sigset_t *set);

/**
 * @brief Examine and change the calling process's signal mask.
 * @ingroup posix_option_group_signals
 * @param how  @c SIG_BLOCK, @c SIG_UNBLOCK, or @c SIG_SETMASK.
 * @param set  Signal set to apply, or NULL.
 * @param oset Output: previous mask, or NULL.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigprocmask.html
 */
int sigprocmask(int how, const sigset_t *ZRESTRICT set, sigset_t *ZRESTRICT oset);

#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)
/**
 * @brief Queue a signal and data to a process.
 * @ingroup posix_option_group_realtime_signals
 * @param pid   Target process ID.
 * @param sig   Signal number.
 * @param value Value to deliver along with the signal.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigqueue.html
 */
int sigqueue(pid_t pid, int sig, union sigval value);
#endif

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/**
 * @brief Remove a signal from the process signal mask (XSI, obsolescent).
 * @ingroup posix_option_group_signals
 * @param sig Signal to unblock.
 * @return 0 on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigrelse.html
 */
int sigrelse(int sig);

/**
 * @brief Set the disposition of a signal, optionally blocking it first (XSI, obsolescent).
 * @ingroup posix_option_group_signals
 * @param sig  Signal number.
 * @param disp New disposition (SIG_DFL, SIG_IGN, SIG_HOLD, or a handler).
 * @return Previous disposition on success, or SIG_ERR on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigset.html
 */
sighandler_t sigset(int sig, sighandler_t disp);
#endif

/**
 * @brief Wait for a signal, atomically replacing the process signal mask.
 * @ingroup posix_option_group_signals
 * @param set New signal mask to apply while waiting.
 * @return Always returns -1 with @c errno set to @c EINTR.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigsuspend.html
 */
int sigsuspend(const sigset_t *set);

#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)
/**
 * @brief Wait for a queued signal with a timeout.
 * @ingroup posix_option_group_realtime_signals
 * @param set     Set of signals to wait for.
 * @param info    Output: information about the accepted signal, or NULL.
 * @param timeout Maximum time to wait.
 * @return Signal number on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigtimedwait.html
 */
int sigtimedwait(const sigset_t *ZRESTRICT set, siginfo_t *ZRESTRICT info,
		 const struct timespec *ZRESTRICT timeout);
#endif

/**
 * @brief Wait for a signal from a set.
 * @ingroup posix_option_group_signals
 * @param set Output signal set to wait on.
 * @param sig Output: number of the accepted signal.
 * @return 0 on success, or a positive error number on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigwait.html
 */
int sigwait(const sigset_t *ZRESTRICT set, int *ZRESTRICT sig);

#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)
/**
 * @brief Wait for a queued signal (no timeout).
 * @ingroup posix_option_group_realtime_signals
 * @param set  Set of signals to wait for.
 * @param info Output: information about the accepted signal, or NULL.
 * @return Signal number on success, or -1 with errno set on failure.
 * @see https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigwaitinfo.html
 */
int sigwaitinfo(const sigset_t *ZRESTRICT set, siginfo_t *ZRESTRICT info);
#endif

/* Note: only ANSI / ISO C signals are guarded below */

#define SIGHUP 1 /**< Hangup */
#if !defined(SIGINT) || defined(__DOXYGEN__)
#define SIGINT 2 /**< Interrupt */
#endif
#define SIGQUIT 3 /**< Quit */
#if !defined(SIGILL) || defined(__DOXYGEN__)
#define SIGILL 4 /**< Illegal instruction */
#endif
#define SIGTRAP 5 /**< Trace/breakpoint trap */
#if !defined(SIGABRT) || defined(__DOXYGEN__)
#define SIGABRT 6 /**< Aborted */
#endif
#define SIGBUS 7 /**< Bus error */
#if !defined(SIGFPE) || defined(__DOXYGEN__)
#define SIGFPE 8 /**< Arithmetic exception */
#endif
#define SIGKILL 9  /**< Killed */
#define SIGUSR1 10 /**< User-defined signal 1 */
#if !defined(SIGSEGV) || defined(__DOXYGEN__)
#define SIGSEGV 11 /**< Invalid memory reference */
#endif
#define SIGUSR2 12 /**< User-defined signal 2 */
#define SIGPIPE 13 /**< Broken pipe */
#define SIGALRM 14 /**< Alarm clock */
#if !defined(SIGTERM) || defined(__DOXYGEN__)
#define SIGTERM 15 /**< Terminated */
#endif
/* 16 not used */
#define SIGCHLD   17 /**< Child status changed */
#define SIGCONT   18 /**< Continued */
#define SIGSTOP   19 /**< Stop executing */
#define SIGTSTP   20 /**< Stopped */
#define SIGTTIN   21 /**< Stopped (read) */
#define SIGTTOU   22 /**< Stopped (write) */
#define SIGURG    23 /**< Urgent I/O condition */
#define SIGXCPU   24 /**< CPU time limit exceeded */
#define SIGXFSZ   25 /**< File size limit exceeded */
#define SIGVTALRM 26 /**< Virtual timer expired */
#define SIGPROF   27 /**< Profiling timer expired */
/* 28 not used */
#define SIGPOLL   29 /**< Pollable event occurred */
/* 30 not used */
#define SIGSYS    31 /**< Bad system call */

#if defined(_POSIX_REALTIME_SIGNALS) || defined(__DOXYGEN__)

/* SIGILL */
#define ILL_ILLOPC 1 /**< Illegal opcode */
#define ILL_ILLOPN 2 /**< Illegal operand */
#define ILL_ILLADR 3 /**< Illegal addressing mode */
#define ILL_ILLTRP 4 /**< Illegal trap */
#define ILL_PRVOPC 5 /**< Privileged opcode */
#define ILL_PRVREG 6 /**< Privileged register */
#define ILL_COPROC 7 /**< Coprocessor error */
#define ILL_BADSTK 8 /**< Internal stack error */

/* SIGFPE */
#define FPE_INTDIV 9  /**< Integer divide by zero */
#define FPE_INTOVF 10 /**< Integer overflow */
#define FPE_FLTDIV 11 /**< Floating-point divide by zero */
#define FPE_FLTOVF 12 /**< Floating-point overflow */
#define FPE_FLTUND 13 /**< Floating-point underflow */
#define FPE_FLTRES 15 /**< Floating-point inexact result */
#define FPE_FLTINV 16 /**< Invalid floating-point operation */
#define FPE_FLTSUB 17 /**< Subscript out of range */

/* SIGSEGV */
#define SEGV_MAPERR 18 /**< Address not mapped to object */
#define SEGV_ACCERR 19 /**< Invalid permissions for mapped object */

/* SIGBUS */
#define BUS_ADRALN 20 /**< Invalid address alignment */
#define BUS_ADRERR 21 /**< Nonexistent physical address */
#define BUS_OBJERR 22 /**< Object-specific hardware error */

#if defined(_XOPEN_SOURCE) || defined(__DOXYGEN__)
/* SIGTRAP */
#define TRAP_BRKPT 23 /**< Process breakpoint */
#define TRAP_TRACE 24 /**< Process trace trap */
#endif

/* SIGCHLD */
#define CLD_EXITED    25 /**< Child has exited */
#define CLD_KILLED    26 /**< Child has terminated abnormally and did not create a core file */
#define CLD_DUMPED    27 /**< Child has terminated abnormally and created a core file */
#define CLD_TRAPPED   28 /**< Traced child has trapped */
#define CLD_STOPPED   29 /**< Child has stopped */
#define CLD_CONTINUED 30 /**< Stopped child has continued */

#if defined(_XOPEN_STREAMS) || defined(__DOXYGEN__)
/* SIGPOLL */
#define POLL_IN  31 /**< Data input available */
#define POLL_OUT 32 /**< Output buffers available */
#define POLL_MSG 33 /**< Input message available */
#define POLL_ERR 34 /**< I/O error */
#define POLL_PRI 35 /**< High priority input available */
#define POLL_HUP 36 /**< Device disconnected */
#endif

/* Any */
#define SI_USER    37 /**< Signal sent by kill() */
#define SI_QUEUE   38 /**< Signal sent by sigqueue() */
#define SI_TIMER   39 /**< Signal generated by expiration of a timer set by timer_settime() */
#define SI_ASYNCIO 40 /**< Signal generated by completion of an asynchronous I/O request */
#define SI_MESGQ   41 /**< Signal generated by arrival of a message on an empty message queue */

#endif


#ifdef __cplusplus
}
#endif

#endif /* defined(_POSIX_C_SOURCE) || defined(__DOXYGEN__) */

#endif /* ZEPHYR_INCLUDE_POSIX_POSIX_SIGNAL_H_ */

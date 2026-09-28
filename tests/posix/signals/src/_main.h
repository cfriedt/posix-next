/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_SIGNALS_MAIN_H_
#define POSIX_TESTS_SIGNALS_MAIN_H_

#include <signal.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

/*
 * Everything except the child-thread termination and fatal-error cases runs as ZTEST_USER(): the
 * option group keeps no state of its own -- dispositions and the alarm live in the kernel,
 * behind system calls. The termination cases stay supervisor-side because they assert on what a
 * dying thread leaves behind, which is kernel business either way.
 */

/* provided by ../common/test_sigmask_common.c */
void test_pthread_sigmask_init_expectation_mask(void);
void test_pthread_sigmask_common(int (*sigmask)(int how, const sigset_t *set, sigset_t *oset));

/*
 * State recorded by test_signals_handler() and test_signals_sigaction_handler()
 *
 * A disposition belongs to the process, not to the caller, so a test case cannot assume anything
 * about what ran before it. Every case that catches a signal resets this first.
 */
struct test_signals_record {
	int calls;         /* number of times the handler has been entered */
	int signo;         /* signal number the handler was passed */
	int si_signo;      /* siginfo_t::si_signo, for SA_SIGINFO handlers */
	int si_code;       /* siginfo_t::si_code, for SA_SIGINFO handlers */
	sigset_t mask;     /* signal mask in effect inside the handler */
	bool mask_valid;   /* whether mask could be read */
};

extern struct test_signals_record test_signals_state;

/* A one-argument handler that records its arguments in test_signals_state */
void test_signals_handler(int signo);

/* A three-argument (SA_SIGINFO) handler that records its arguments */
void test_signals_sigaction_handler(int signo, siginfo_t *info, void *context);

/* Forget everything test_signals_handler() has recorded so far */
void test_signals_record_reset(void);

/*
 * Restore signo to its default disposition, unblock it, and discard it if pending
 *
 * Dispositions, the signal mask, and pending signals all outlive the test case that created
 * them, so each case starts from a known state by calling this for every signal it touches.
 */
void test_signals_reset(int signo);

/*
 * Install the default disposition for signo
 *
 * Only the cases that assert on the default disposition need it, since test_signals_reset()
 * deliberately leaves the signal ignored where SIG_DFL would be dangerous.
 */
void test_signals_set_default(int signo);

/* Install handler for signo with an empty mask and flags, and reset the record */
void test_signals_install(int signo, void *handler, int flags);

/* Whether signo is currently blocked for the calling thread */
bool test_signals_blocked(int signo);

/*
 * Wait, up to a couple of seconds, for the handler to have been entered calls times
 *
 * A signal directed at the process rather than at a thread is delivered to whichever thread is
 * willing to take it, and not necessarily before the call that generated it returns.
 *
 * returns true if the handler has run at least calls times
 */
bool test_signals_wait_for_delivery(int calls);

/*
 * Skip the running test case when the C library owns the implementation under test
 *
 * The linux_compat variant runs these expectations against the host C library. Cases that would
 * end the process there, such as those that let a signal's default action run, opt out.
 */
#define TEST_SIGNALS_SKIP_IF_LINUX_COMPAT()                                                        \
	do {                                                                                       \
		if (IS_ENABLED(CONFIG_POSIX_TEST_LINUX_COMPAT)) {                                  \
			ztest_test_skip();                                                         \
		}                                                                                  \
	} while (false)

/* Stack size for the child threads used by cases whose signal terminates the caller */
#define TEST_SIGNALS_CHILD_STACK_SIZE 2048

#endif /* POSIX_TESTS_SIGNALS_MAIN_H_ */

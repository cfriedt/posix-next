/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "_main.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

ZTEST_USER(posix_signals, test_sigpending)
{
	int accepted = 0;
	sigset_t set;
	sigset_t pending;
	sigset_t previous;

	test_signals_reset(SIGUSR1);

	/* nothing is pending to begin with */
	zassert_ok(sigfillset(&pending));
	zassert_ok(sigpending(&pending));
	zassert_equal(0, sigismember(&pending, SIGUSR1), "%s was pending before it was raised",
		      "SIGUSR1");

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGUSR1));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, &previous));

	/* a signal that is blocked when generated is reported as pending */
	zassert_ok(raise(SIGUSR1));
	zassert_ok(sigpending(&pending));
	zassert_equal(1, sigismember(&pending, SIGUSR1),
		      "a blocked signal that was raised is not reported as pending");

	/* signals that were never generated are not */
	zassert_equal(0, sigismember(&pending, SIGUSR2));

	/* accepting the signal clears it from the pending set */
	zassert_ok(sigwait(&set, &accepted));
	zassert_equal(SIGUSR1, accepted);

	zassert_ok(sigpending(&pending));
	zassert_equal(0, sigismember(&pending, SIGUSR1),
		      "%s is still pending after it was accepted", "SIGUSR1");

	zassert_ok(sigprocmask(SIG_SETMASK, &previous, NULL));
}

ZTEST_USER(posix_signals, test_sigwait)
{
	int sig;
	sigset_t set;
	sigset_t pending;
	sigset_t previous;

	test_signals_reset(SIGUSR1);
	test_signals_reset(SIGUSR2);

	/*
	 * POSIX requires the signals in set to be blocked at the time of the call; the behaviour
	 * is undefined otherwise.
	 *
	 * TODO (processes): when several threads wait on the same signal, exactly one must return
	 * from sigwait(), and a signal generated for a specific thread must go to that thread.
	 * Neither is meaningful until signals are process-scoped.
	 */
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGUSR1));
	zassert_ok(sigaddset(&set, SIGUSR2));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, &previous));

	/* an already pending signal is returned without suspending the caller */
	zassert_ok(raise(SIGUSR1));

	sig = 0;
	zassert_ok(sigwait(&set, &sig), "sigwait() reports errors through its return value");
	zassert_equal(SIGUSR1, sig, "sigwait() accepted signal %d, expected %d", sig, SIGUSR1);

	/* the accepted signal is atomically cleared from the set of pending signals */
	zassert_ok(sigpending(&pending));
	zassert_equal(0, sigismember(&pending, SIGUSR1),
		      "sigwait() did not clear the accepted signal from the pending set");

	/* only signals in set are accepted; the others stay pending */
	zassert_ok(raise(SIGUSR2));
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGUSR1));

	zassert_ok(raise(SIGUSR1));
	sig = 0;
	zassert_ok(sigwait(&set, &sig));
	zassert_equal(SIGUSR1, sig);

	zassert_ok(sigpending(&pending));
	zassert_equal(1, sigismember(&pending, SIGUSR2),
		      "sigwait() accepted a signal that was not in set");

	zassert_ok(sigprocmask(SIG_SETMASK, &previous, NULL));
	test_signals_reset(SIGUSR1);
	test_signals_reset(SIGUSR2);
}

static void *waker_entry(void *sleeper)
{
	/*
	 * The signal is directed at the thread blocked in pause() rather than at the process, so
	 * that it cannot be taken by some other thread.
	 */
	k_msleep(100);
	(void)pthread_kill((pthread_t)(uintptr_t)sleeper, SIGUSR1);

	return NULL;
}

ZTEST_USER(posix_signals, test_pause)
{
	pthread_t waker;

	test_signals_reset(SIGUSR1);
	test_signals_install(SIGUSR1, test_signals_handler, 0);

	/*
	 * pause() suspends the caller until a signal is delivered whose action is either to
	 * terminate it or to invoke a handler, so arrange for one to arrive.
	 */
	zassert_ok(pthread_create(&waker, NULL, waker_entry,
				  (void *)(uintptr_t)pthread_self()));

	/* pause() never returns successfully */
	zassert_equal(-1, pause(), "pause() must always return -1");
	zassert_equal(EINTR, errno);

	zassert_equal(1, test_signals_state.calls, "pause() returned without running a handler");
	zassert_equal(SIGUSR1, test_signals_state.signo);

	zassert_ok(pthread_join(waker, NULL));
	test_signals_reset(SIGUSR1);
}

ZTEST_USER(posix_signals, test_sigsuspend)
{
	sigset_t empty;
	sigset_t blocked;
	sigset_t previous;
	sigset_t restored;

	test_signals_reset(SIGUSR1);
	test_signals_install(SIGUSR1, test_signals_handler, 0);

	/*
	 * Raise SIGUSR1 while it is blocked so that it is pending, then let sigsuspend() install a
	 * mask that unblocks it. The handler must run before sigsuspend() returns.
	 */
	zassert_ok(sigemptyset(&blocked));
	zassert_ok(sigaddset(&blocked, SIGUSR1));
	zassert_ok(sigprocmask(SIG_BLOCK, &blocked, &previous));

	zassert_ok(raise(SIGUSR1));
	zassert_equal(0, test_signals_state.calls, "a blocked signal was delivered");

	zassert_ok(sigemptyset(&empty));
	zassert_equal(-1, sigsuspend(&empty), "sigsuspend() must always return -1");
	zassert_equal(EINTR, errno);

	zassert_equal(1, test_signals_state.calls, "sigsuspend() returned without running a handler");
	zassert_equal(SIGUSR1, test_signals_state.signo);

	/* the mask in force before the call is restored before it returns */
	zassert_ok(sigprocmask(SIG_SETMASK, NULL, &restored));
	zassert_equal(1, sigismember(&restored, SIGUSR1),
		      "sigsuspend() did not restore the previous signal mask");

	zassert_ok(sigprocmask(SIG_SETMASK, &previous, NULL));
	test_signals_reset(SIGUSR1);
}

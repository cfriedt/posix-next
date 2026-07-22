/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "_main.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

/*
 * A signal whose default action terminates ends the thread it is delivered to, so cases that let
 * one run do so in a child thread and inspect what is left behind once it is gone.
 *
 * TODO (processes): once Zephyr has processes, the default action should end the process rather
 * than the one thread, and these cases should assert that instead.
 */

static ZTEST_BMEM volatile bool child_survived;
static pthread_t child_thread;

static int spawn_child(void *(*entry)(void *), void *arg)
{
	child_survived = false;

	return pthread_create(&child_thread, NULL, entry, arg);
}

ZTEST_USER(posix_signals, test_kill)
{
	test_signals_reset(SIGUSR1);

	/* [EINVAL] the sig argument is not a valid signal number */
	zassert_equal(-1, kill(getpid(), SIGRTMAX + 1));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, kill(getpid(), -1));
	zassert_equal(EINVAL, errno);

	/*
	 * TODO (processes): kill() can only name the calling thread, via getpid(), or a specific
	 * thread, via a pthread_t cast to pid_t. Delivery to another process, to a process group
	 * (pid <= 0), and the ESRCH/EPERM cases that go with them are untestable until Zephyr has
	 * processes.
	 */

	/* signal 0 performs error checking but delivers nothing */
	test_signals_install(SIGUSR1, test_signals_handler, 0);
	zassert_ok(kill(getpid(), 0));
	zassert_equal(0, test_signals_state.calls, "signal 0 was delivered");

	/* a signal sent to the calling process invokes its handler */
	zassert_ok(kill(getpid(), SIGUSR1));
	zassert_true(test_signals_wait_for_delivery(1), "the handler ran %d times, expected 1",
		     test_signals_state.calls);
	zassert_equal(SIGUSR1, test_signals_state.signo);

	/*
	 * Blocking is not exercised here. It is a property of the calling thread, and kill() is
	 * directed at the process, so holding a signal pending this way only works where the two
	 * are the same. test_raise() covers it with a thread-directed signal instead.
	 */

	test_signals_reset(SIGUSR1);
}

ZTEST_USER(posix_signals, test_raise)
{
	sigset_t blocked;
	sigset_t pending;

	test_signals_reset(SIGUSR1);

	/* [EINVAL] the sig argument is not a valid signal number */
	zassert_equal(-1, raise(SIGRTMAX + 1));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, raise(-1));
	zassert_equal(EINVAL, errno);

	test_signals_install(SIGUSR1, test_signals_handler, 0);

	/* the handler completes before raise() returns */
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls,
		      "the handler ran %d times before raise() returned, expected 1",
		      test_signals_state.calls);
	zassert_equal(SIGUSR1, test_signals_state.signo);

	zassert_ok(raise(SIGUSR1));
	zassert_equal(2, test_signals_state.calls);

	/* a raised signal that is blocked is held pending */
	zassert_ok(sigemptyset(&blocked));
	zassert_ok(sigaddset(&blocked, SIGUSR1));
	zassert_ok(sigprocmask(SIG_BLOCK, &blocked, NULL));

	zassert_ok(raise(SIGUSR1));
	zassert_equal(2, test_signals_state.calls, "a blocked signal was delivered");
	zassert_ok(sigpending(&pending));
	zassert_equal(1, sigismember(&pending, SIGUSR1));

	zassert_ok(sigprocmask(SIG_UNBLOCK, &blocked, NULL));
	zassert_equal(3, test_signals_state.calls);

	test_signals_reset(SIGUSR1);
}

ZTEST_USER(posix_signals, test_alarm)
{
	test_signals_reset(SIGALRM);

	/* with no alarm scheduled there is no remaining time to report */
	zassert_equal(0, alarm(0));

	/* scheduling one reports what was left of the previous one, and cancels it */
	zassert_equal(0, alarm(10));
	zassert_equal(10, alarm(5), "the remaining time of a pending alarm was not reported");
	zassert_equal(5, alarm(0), "a rescheduled alarm did not replace the previous one");
	zassert_equal(0, alarm(0), "alarm(0) did not cancel the pending alarm");

	/* an alarm that expires generates SIGALRM for the process */
	test_signals_install(SIGALRM, test_signals_handler, 0);
	zassert_equal(0, alarm(1));

	zassert_true(test_signals_wait_for_delivery(1), "alarm() did not generate %s", "SIGALRM");
	zassert_equal(SIGALRM, test_signals_state.signo);

	zassert_equal(0, alarm(0));
	test_signals_reset(SIGALRM);
}

/*
 * The child installs the disposition itself: Zephyr has no processes, so a signal action is
 * associated with the thread that installed it rather than with the process as POSIX requires.
 */
static void *abort_entry(void *handler)
{
	struct sigaction act = {
		.sa_handler = (void (*)(int))handler,
		.sa_flags = 0,
	};

	if (handler != NULL) {
		(void)sigemptyset(&act.sa_mask);
		(void)sigaction(SIGABRT, &act, NULL);
	}

	abort();

	child_survived = true;

	return NULL;
}

ZTEST(posix_signals, test_abort)
{
	/* the host C library's abort() would take the whole test binary with it */
	TEST_SIGNALS_SKIP_IF_LINUX_COMPAT();

	test_signals_reset(SIGABRT);

	/* abort() does not return to its caller */
	zassert_ok(spawn_child(abort_entry, NULL));
	zassert_ok(pthread_join(child_thread, NULL),
		   "abort() did not terminate the calling thread");
	zassert_false(child_survived, "abort() returned to its caller");

	/* abort() raises SIGABRT, so an installed handler runs */
	test_signals_record_reset();

	zassert_ok(spawn_child(abort_entry, test_signals_handler));
	zassert_ok(pthread_join(child_thread, NULL));

	zassert_equal(1, test_signals_state.calls, "abort() did not raise %s", "SIGABRT");
	zassert_equal(SIGABRT, test_signals_state.signo);

	/*
	 * and it terminates the caller even though the handler returned, by restoring the default
	 * disposition and raising SIGABRT a second time.
	 */
	zassert_false(child_survived, "abort() returned after its handler returned");

	/* ignoring SIGABRT does not make abort() a no-op: it still terminates the caller */
	zassert_ok(spawn_child(abort_entry, SIG_IGN));
	zassert_ok(pthread_join(child_thread, NULL));
	zassert_false(child_survived, "abort() returned with %s ignored", "SIGABRT");

	test_signals_reset(SIGABRT);
}

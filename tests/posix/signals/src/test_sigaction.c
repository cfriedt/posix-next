/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "_main.h"

#include <errno.h>
#include <signal.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

static void other_handler(int signo)
{
	ARG_UNUSED(signo);
}

ZTEST_USER(posix_signals, test_sigaction)
{
	sigset_t unmaskable;
	struct sigaction oact;
	struct sigaction act = {
		.sa_handler = test_signals_handler,
		.sa_flags = 0,
	};

	zassert_ok(sigemptyset(&act.sa_mask));

	/* [EINVAL] the sig argument is not a valid signal number */
	zassert_equal(-1, sigaction(0, &act, NULL));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaction(-1, &act, NULL));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaction(SIGRTMAX + 1, &act, NULL));
	zassert_equal(EINVAL, errno);

	/*
	 * The two signals reserved for thread cancellation are not available to applications.
	 * Zephyr follows the Linux NPTL implementation here.
	 */
	zassert_equal(-1, sigaction(CONFIG_THREAD_CANCEL_SIGNAL_NUMBER, &act, NULL));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaction(CONFIG_THREAD_CANCEL_SIGNAL_NUMBER + 1, &act, NULL));
	zassert_equal(EINVAL, errno);

	/* [EINVAL] an attempt to catch a signal that cannot be caught */
	zassert_equal(-1, sigaction(SIGKILL, &act, NULL));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaction(SIGSTOP, &act, NULL));
	zassert_equal(EINVAL, errno);

	/* [EINVAL] an attempt to ignore a signal that cannot be ignored */
	act.sa_handler = SIG_IGN;
	zassert_equal(-1, sigaction(SIGKILL, &act, NULL));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaction(SIGSTOP, &act, NULL));
	zassert_equal(EINVAL, errno);

	test_signals_reset(SIGUSR1);
	test_signals_reset(SIGUSR2);

	/*
	 * The default disposition is checked on a signal that nothing here generates. Leaving
	 * SIG_DFL installed for a signal that is in play would let a stray instance run the
	 * default action, which ends the process, and blocking it would not help: a signal aimed
	 * at the process is taken by whichever thread will have it.
	 */
	test_signals_set_default(SIGVTALRM);

	/* a NULL act queries the current action without changing it */
	zassert_ok(sigaction(SIGVTALRM, NULL, &oact));
	zassert_equal(SIG_DFL, oact.sa_handler, "the default disposition was not reported");
	zassert_ok(sigaction(SIGVTALRM, NULL, &oact));
	zassert_equal(SIG_DFL, oact.sa_handler, "querying the action changed it");

	/* oact reports the action that was in effect before the call */
	act.sa_handler = test_signals_handler;
	zassert_ok(sigaction(SIGVTALRM, &act, &oact));
	zassert_equal(SIG_DFL, oact.sa_handler);
	test_signals_reset(SIGVTALRM);

	act.sa_handler = test_signals_handler;
	zassert_ok(sigaction(SIGUSR1, &act, NULL));

	zassert_ok(sigaction(SIGUSR1, NULL, &oact));
	zassert_equal(test_signals_handler, oact.sa_handler,
		      "the installed handler was not reported back");

	act.sa_handler = other_handler;
	zassert_ok(sigaction(SIGUSR1, &act, &oact));
	zassert_equal(test_signals_handler, oact.sa_handler);

	act.sa_handler = SIG_IGN;
	zassert_ok(sigaction(SIGUSR1, &act, &oact));
	zassert_equal(other_handler, oact.sa_handler);

	/* an ignored signal is discarded rather than delivered */
	test_signals_record_reset();
	zassert_ok(raise(SIGUSR1));
	zassert_equal(0, test_signals_state.calls, "an ignored signal invoked a handler");

	/*
	 * TODO (processes): POSIX associates an action with the process, but Zephyr keys it by
	 * (signal, thread), so nothing here can check that an action installed by one thread is in
	 * force for another. That becomes testable once processes exist.
	 */

	/* a caught signal invokes the handler with the signal number */
	test_signals_install(SIGUSR1, test_signals_handler, 0);
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls, "the handler ran %d times, expected 1",
		      test_signals_state.calls);
	zassert_equal(SIGUSR1, test_signals_state.signo);

	/* the action stays installed until it is explicitly changed */
	zassert_ok(raise(SIGUSR1));
	zassert_equal(2, test_signals_state.calls, "the action did not stay installed");

	/*
	 * The mask in force inside the handler is the union of the mask before delivery, sa_mask,
	 * and the signal being delivered.
	 */
	act.sa_handler = test_signals_handler;
	act.sa_flags = 0;
	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaddset(&act.sa_mask, SIGUSR2));
	zassert_ok(sigaction(SIGUSR1, &act, NULL));

	test_signals_record_reset();
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls);
	zassert_true(test_signals_state.mask_valid, "the mask could not be read in the handler");
	zassert_equal(1, sigismember(&test_signals_state.mask, SIGUSR2),
		      "sa_mask did not block %s inside the handler", "SIGUSR2");
	zassert_equal(1, sigismember(&test_signals_state.mask, SIGUSR1),
		      "the signal being delivered was not blocked inside the handler");

	/* and the mask in force before delivery is restored when the handler returns */
	zassert_false(test_signals_blocked(SIGUSR1),
		      "the mask was not restored after the handler returned");
	zassert_false(test_signals_blocked(SIGUSR2),
		      "the mask was not restored after the handler returned");

	/* SA_NODEFER leaves the signal being delivered unblocked inside the handler */
	act.sa_flags = SA_NODEFER;
	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(SIGUSR1, &act, NULL));

	test_signals_record_reset();
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls);
	zassert_equal(0, sigismember(&test_signals_state.mask, SIGUSR1),
		      "SA_NODEFER did not leave %s unblocked inside the handler", "SIGUSR1");

	/* SA_RESETHAND restores the default disposition on entry to the handler */
	act.sa_flags = SA_RESETHAND;
	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(SIGUSR1, &act, NULL));

	test_signals_record_reset();
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls);
	zassert_ok(sigaction(SIGUSR1, NULL, &oact));
	zassert_equal(SIG_DFL, oact.sa_handler,
		      "SA_RESETHAND did not restore the default disposition");

	/* SA_SIGINFO selects the three-argument handler and supplies a siginfo_t */
	act.sa_sigaction = test_signals_sigaction_handler;
	act.sa_flags = SA_SIGINFO;
	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(SIGUSR1, &act, NULL));

	test_signals_record_reset();
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls);
	zassert_equal(SIGUSR1, test_signals_state.signo);
	zassert_equal(SIGUSR1, test_signals_state.si_signo,
		      "siginfo_t::si_signo was %d, expected %d", test_signals_state.si_signo,
		      SIGUSR1);

	/*
	 * SIGKILL and SIGSTOP cannot be added to the mask through sa_mask, and the restriction is
	 * enforced without indicating an error.
	 */
	zassert_ok(sigemptyset(&unmaskable));
	zassert_ok(sigaddset(&unmaskable, SIGKILL));
	zassert_ok(sigaddset(&unmaskable, SIGSTOP));

	/* a kernel thread blocks all signals by default, so start from a known state */
	zassert_ok(sigprocmask(SIG_UNBLOCK, &unmaskable, NULL));

	act.sa_handler = test_signals_handler;
	act.sa_flags = 0;
	act.sa_mask = unmaskable;
	zassert_ok(sigaction(SIGUSR1, &act, NULL), "sa_mask containing %s was rejected",
		   "SIGKILL/SIGSTOP");

	test_signals_record_reset();
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls);
	zassert_equal(0, sigismember(&test_signals_state.mask, SIGKILL),
		      "%s must not be blockable through sa_mask", "SIGKILL");
	zassert_equal(0, sigismember(&test_signals_state.mask, SIGSTOP),
		      "%s must not be blockable through sa_mask", "SIGSTOP");

	test_signals_reset(SIGUSR1);
	test_signals_reset(SIGUSR2);
}

ZTEST_USER(posix_signals, test_signal)
{
	sigset_t blocked;
	sigset_t previous;
	struct sigaction oact;

	/* SIG_ERR with EINVAL for signals that can neither be caught nor ignored */
	zassert_equal(SIG_ERR, signal(SIGKILL, test_signals_handler));
	zassert_equal(EINVAL, errno);
	zassert_equal(SIG_ERR, signal(SIGSTOP, test_signals_handler));
	zassert_equal(EINVAL, errno);

	/* and for signal numbers that are not valid */
	zassert_equal(SIG_ERR, signal(SIGRTMAX + 1, test_signals_handler));
	zassert_equal(EINVAL, errno);
	zassert_equal(SIG_ERR, signal(0, test_signals_handler));
	zassert_equal(EINVAL, errno);

	test_signals_reset(SIGUSR1);

	/*
	 * Block the signal while its disposition is being shuffled: the sequence below leaves it
	 * at SIG_DFL for a moment, and an instance generated meanwhile would run the default
	 * action, which on a hosted C library ends the test binary.
	 */
	zassert_ok(sigemptyset(&blocked));
	zassert_ok(sigaddset(&blocked, SIGUSR1));
	zassert_ok(sigprocmask(SIG_BLOCK, &blocked, &previous));

	/* the previous disposition is returned, starting from the default */
	test_signals_set_default(SIGVTALRM);
	zassert_equal(SIG_DFL, signal(SIGVTALRM, test_signals_handler), "expected %s", "SIG_DFL");
	test_signals_reset(SIGVTALRM);

	(void)signal(SIGUSR1, test_signals_handler);
	zassert_equal(test_signals_handler, signal(SIGUSR1, other_handler),
		      "the previously installed handler was not reported");
	zassert_equal(other_handler, signal(SIGUSR1, SIG_IGN));

	/*
	 * The chain deliberately ends on a handler rather than on SIG_DFL: leaving the disposition
	 * at SIG_DFL, even briefly, lets a stray instance run the default action, and blocking the
	 * signal here would not help because a process-directed instance can be taken by any thread.
	 */
	zassert_equal(SIG_IGN, signal(SIGUSR1, test_signals_handler));

	/*
	 * POSIX specifies signal() in terms of sigaction(), so a disposition installed by one is
	 * visible to the other.
	 */
	zassert_ok(sigprocmask(SIG_SETMASK, &previous, NULL));
	zassert_ok(sigaction(SIGUSR1, NULL, &oact));
	zassert_equal(test_signals_handler, oact.sa_handler,
		      "sigaction() did not report the handler installed by signal()");

	/* a handler installed by signal() is invoked on delivery */
	test_signals_record_reset();
	zassert_ok(raise(SIGUSR1));
	zassert_equal(1, test_signals_state.calls, "the handler ran %d times, expected 1",
		      test_signals_state.calls);
	zassert_equal(SIGUSR1, test_signals_state.signo);

	/*
	 * Whether the disposition survives delivery is not asserted: ISO C leaves it
	 * implementation-defined (C17 7.14.1.1), and glibc built with strict _POSIX_C_SOURCE
	 * resets it to SIG_DFL, System V style. Persistence is only guaranteed for a disposition
	 * installed by sigaction() without SA_RESETHAND, which test_sigaction() covers.
	 */

	test_signals_reset(SIGUSR1);
}

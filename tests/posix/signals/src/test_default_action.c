/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "_main.h"

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include <zephyr/fatal.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>
#include <zephyr/ztest_error_hook.h>

/*
 * These two cases cover behaviour rather than a single POSIX function: what happens when nobody
 * catches a signal, and what a thread leaves behind when it dies. Neither outcome is observable
 * from the thread it happens to, so the offending code runs in a child thread.
 */

static pthread_t child_thread;

static ZTEST_BMEM volatile bool child_survived;
static ZTEST_BMEM int child_signo;
static ZTEST_BMEM bool child_ignores;
static ZTEST_BMEM volatile int fatal_reason;
static ZTEST_BMEM volatile int fatal_count;

void ztest_post_fatal_error_hook(unsigned int reason, const struct arch_esf *esf)
{
	ARG_UNUSED(esf);

	fatal_reason = (int)reason;
	fatal_count++;
}

static int spawn_child(void *(*entry)(void *), int signo, bool ignores)
{
	child_survived = false;
	child_signo = signo;
	child_ignores = ignores;

	return pthread_create(&child_thread, NULL, entry, NULL);
}

static void *raise_entry(void *arg)
{
	sigset_t empty;
	struct sigaction act = {
		.sa_handler = SIG_IGN,
		.sa_flags = 0,
	};

	ARG_UNUSED(arg);

	/* a kernel thread blocks all signals by default, and must opt in to delivery */
	(void)sigemptyset(&empty);
	(void)sigprocmask(SIG_SETMASK, &empty, NULL);

	if (child_ignores) {
		(void)sigemptyset(&act.sa_mask);
		(void)sigaction(child_signo, &act, NULL);
	}

	(void)raise(child_signo);

	child_survived = true;

	return NULL;
}

ZTEST(posix_signals, test_default_action)
{
	/* on a hosted C library these default actions would end the test binary */
	TEST_SIGNALS_SKIP_IF_LINUX_COMPAT();

	test_signals_reset(SIGTERM);
	test_signals_reset(SIGCHLD);

	/* the default action for SIGTERM is to terminate */
	zassert_ok(spawn_child(raise_entry, SIGTERM, false));
	zassert_ok(pthread_join(child_thread, NULL),
		   "the default action for %s did not terminate the thread", "SIGTERM");
	zassert_false(child_survived, "the thread ran on past a signal that should have ended it");

	/* the default action for SIGCHLD is to ignore */
	zassert_ok(spawn_child(raise_entry, SIGCHLD, false));
	zassert_ok(pthread_join(child_thread, NULL));
	zassert_true(child_survived, "the default action for %s terminated the thread", "SIGCHLD");

	/* and an explicitly ignored signal does not terminate either */
	zassert_ok(spawn_child(raise_entry, SIGTERM, true));
	zassert_ok(pthread_join(child_thread, NULL));
	zassert_true(child_survived, "an explicitly ignored %s terminated the thread", "SIGTERM");
}

static void *oops_entry(void *arg)
{
	ARG_UNUSED(arg);

	ztest_set_fault_valid(true);
	k_oops();

	child_survived = true;

	return NULL;
}

/*
 * The cause reported for a fatal error, and its effect on the offending thread
 *
 * Zephyr reports CPU exceptions and kernel errors through the fatal error path rather than by
 * generating SIGILL, SIGFPE, SIGSEGV, or SIGBUS for the offending thread. This case pins the
 * behaviour that exists today; once the kernel does generate those signals, it should assert on
 * the signal instead.
 */
ZTEST(posix_signals, test_fatal_error)
{
	TEST_SIGNALS_SKIP_IF_LINUX_COMPAT();

	fatal_reason = -1;
	fatal_count = 0;

	zassert_ok(spawn_child(oops_entry, 0, false));
	zassert_ok(pthread_join(child_thread, NULL));

	zassert_equal(1, fatal_count, "the fatal error hook ran %d times, expected 1", fatal_count);
	zassert_equal(K_ERR_KERNEL_OOPS, fatal_reason, "fatal error reason was %d, expected %d",
		      fatal_reason, K_ERR_KERNEL_OOPS);
	zassert_false(child_survived, "the thread ran on past a fatal error");
}

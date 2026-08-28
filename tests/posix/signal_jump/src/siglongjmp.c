/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <setjmp.h>
#include <signal.h>

#include <zephyr/ztest.h>

static ZTEST_BMEM sigjmp_buf jump_env;
static ZTEST_BMEM volatile sig_atomic_t handler_jumps;

static void jump_from_nested_frame(int val)
{
	siglongjmp(jump_env, val);
}

static void siglongjmp_returns_value(void)
{
	int r;
	volatile bool reached = false;

	r = sigsetjmp(jump_env, 0);
	if (r == 0) {
		reached = true;
		jump_from_nested_frame(42);
		zassert_unreachable("siglongjmp() returned");
	}
	zassert_equal(r, 42);
	zassert_true(reached);

	/* a val of 0 is returned as 1 */
	r = sigsetjmp(jump_env, 0);
	if (r == 0) {
		jump_from_nested_frame(0);
		zassert_unreachable("siglongjmp() returned");
	}
	zassert_equal(r, 1);
}

static void siglongjmp_restores_mask(void)
{
	sigset_t set;
	sigset_t cur;

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGUSR2));
	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));

	/* the mask saved by sigsetjmp(..., 1) is restored by siglongjmp() */
	if (sigsetjmp(jump_env, 1) == 0) {
		zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
		jump_from_nested_frame(1);
	}
	zassert_ok(sigprocmask(SIG_SETMASK, NULL, &cur));
	zassert_equal(sigismember(&cur, SIGUSR2), 0);

	/* with a savemask of 0, siglongjmp() leaves the mask alone */
	if (sigsetjmp(jump_env, 0) == 0) {
		zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
		jump_from_nested_frame(1);
	}
	zassert_ok(sigprocmask(SIG_SETMASK, NULL, &cur));
	zassert_equal(sigismember(&cur, SIGUSR2), 1);

	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));
}

static void siglongjmp_handler(int signo)
{
	ARG_UNUSED(signo);

	++handler_jumps;
	siglongjmp(jump_env, handler_jumps);
}

static void siglongjmp_out_of_handler(void)
{
	sigset_t set;
	struct sigaction oact;
	struct sigaction act = {
		.sa_handler = siglongjmp_handler,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(SIGUSR1, &act, &oact));
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGUSR1));
	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));

	/*
	 * Delivery blocks the signal until the handler returns, but the
	 * handler jumps out instead, so only siglongjmp() restoring the saved
	 * mask makes SIGUSR1 deliverable again for the second raise(); were
	 * the mask not restored, the second raise() would leave the signal
	 * pending and return, reaching zassert_unreachable().
	 */
	handler_jumps = 0;
	if (sigsetjmp(jump_env, 1) < 2) {
		zassert_ok(raise(SIGUSR1));
		zassert_unreachable("SIGUSR1 was not delivered");
	}
	zassert_equal(handler_jumps, 2);

	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
	zassert_ok(sigaction(SIGUSR1, &oact, NULL));
}

ZTEST_USER(posix_signal_jump, test_siglongjmp)
{
	siglongjmp_returns_value();
	siglongjmp_restores_mask();
	siglongjmp_out_of_handler();
}

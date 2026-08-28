/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <setjmp.h>
#include <signal.h>

#include <zephyr/ztest.h>

static void sigsetjmp_direct_return(void)
{
	sigjmp_buf env;

	/* returns 0 when returning directly, with and without saving the mask */
	zassert_ok(sigsetjmp(env, 0));
	zassert_ok(sigsetjmp(env, 1));
}

static void sigsetjmp_leaves_mask(void)
{
	sigjmp_buf env;
	sigset_t before;
	sigset_t after;
	static const int signos[] = {SIGUSR1, SIGUSR2, SIGALRM, SIGTERM};

	/* saving the mask does not modify it */
	zassert_ok(sigprocmask(SIG_SETMASK, NULL, &before));
	zassert_ok(sigsetjmp(env, 1));
	zassert_ok(sigprocmask(SIG_SETMASK, NULL, &after));
	ARRAY_FOR_EACH(signos, i) {
		zassert_equal(sigismember(&before, signos[i]), sigismember(&after, signos[i]));
	}
}

ZTEST_USER(posix_signal_jump, test_sigsetjmp)
{
	sigsetjmp_direct_return();
	sigsetjmp_leaves_mask();
}

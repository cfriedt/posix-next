/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The XSI Signals option group stands alone: these tests install, generate and observe signals
 * with its own functions, except for the delivery on an alternate stack, which only sigaction()
 * (POSIX_SIGNALS) can request.
 */

#include <errno.h>
#include <signal.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

ZTEST_BMEM static volatile int handled;
ZTEST_BMEM static volatile uintptr_t handler_local;
ZTEST_BMEM static uint8_t __aligned(16) altstack[2 * MINSIGSTKSZ];

static void handler(int sig)
{
	volatile int local;

	handler_local = (uintptr_t)&local;
	handled = sig;
}

static void handler_install(int sig)
{
	handled = 0;
	zassert_not_equal(sigset(sig, handler), SIG_ERR);
}

static void disposition_reset(int sig)
{
	zassert_not_equal(sigset(sig, SIG_DFL), SIG_ERR);
}

ZTEST(xsi_signals, test_killpg)
{
	errno = 0;
	zassert_equal(killpg(-1, SIGUSR1), -1);
	zassert_equal(errno, EINVAL);

	errno = 0;
	zassert_equal(killpg(0, -1), -1);
	zassert_equal(errno, EINVAL);

	/* 0: the caller's own process group */
	zassert_ok(killpg(0, 0));
	handler_install(SIGUSR1);
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, SIGUSR1);
	disposition_reset(SIGUSR1);
}

static void sigaltstack_errors(void)
{
	stack_t ss = {.ss_sp = altstack, .ss_size = sizeof(altstack), .ss_flags = SS_ONSTACK};

	errno = 0;
	zassert_equal(sigaltstack(&ss, NULL), -1);
	zassert_equal(errno, EINVAL);

	ss = (stack_t){.ss_sp = altstack, .ss_size = MINSIGSTKSZ - 1};
	errno = 0;
	zassert_equal(sigaltstack(&ss, NULL), -1);
	zassert_equal(errno, ENOMEM);
}

static bool on_altstack(uintptr_t addr)
{
	return (addr >= (uintptr_t)altstack) && (addr < (uintptr_t)altstack + sizeof(altstack));
}

#ifdef CONFIG_POSIX_SIGNALS
static void sigaltstack_delivery(void)
{
	struct sigaction act = {.sa_handler = handler, .sa_flags = SA_ONSTACK};

	handled = 0;
	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(SIGUSR1, &act, NULL));
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, SIGUSR1);
	zassert_true(on_altstack(handler_local), "handler ran at %p", (void *)handler_local);
}
#endif /* CONFIG_POSIX_SIGNALS */

static void sigaltstack_user(void)
{
	stack_t ss = {.ss_sp = altstack, .ss_size = sizeof(altstack)};
	stack_t oss;

	zassert_ok(sigaltstack(NULL, &oss));
	zassert_equal(oss.ss_flags, SS_DISABLE);

	zassert_ok(sigaltstack(&ss, NULL));
	zassert_ok(sigaltstack(NULL, &oss));
	zassert_equal(oss.ss_sp, (void *)altstack);
	zassert_equal(oss.ss_size, sizeof(altstack));
	zassert_equal(oss.ss_flags, 0);

	/* a handler installed without SA_ONSTACK runs where the thread runs */
	handler_install(SIGUSR1);
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, SIGUSR1);
	zassert_false(on_altstack(handler_local));

#ifdef CONFIG_POSIX_SIGNALS
	sigaltstack_delivery();
#endif /* CONFIG_POSIX_SIGNALS */
	disposition_reset(SIGUSR1);

	ss.ss_flags = SS_DISABLE;
	zassert_ok(sigaltstack(&ss, NULL));
	zassert_ok(sigaltstack(NULL, &oss));
	zassert_equal(oss.ss_flags, SS_DISABLE);
}

ZTEST_USER(xsi_signals, test_sigaltstack)
{
	sigaltstack_errors();
	if (IS_ENABLED(CONFIG_SIGNAL_ALTSTACK) && k_is_user_context()) {
		sigaltstack_user();
	} else {
		/* only user threads take signals on an alternate stack */
		stack_t oss;

		errno = 0;
		zassert_equal(sigaltstack(NULL, &oss), -1);
		zassert_equal(errno, ENOTSUP);
	}
}

ZTEST(xsi_signals, test_sighold)
{
	errno = 0;
	zassert_equal(sighold(-1), -1);
	zassert_equal(errno, EINVAL);

	handler_install(SIGUSR1);
	zassert_ok(sighold(SIGUSR1));
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, 0, "delivered while held");
	/* it pends, and is delivered when released */
	zassert_ok(sigrelse(SIGUSR1));
	zassert_equal(handled, SIGUSR1);
	disposition_reset(SIGUSR1);
}

ZTEST(xsi_signals, test_sigignore)
{
	errno = 0;
	zassert_equal(sigignore(-1), -1);
	zassert_equal(errno, EINVAL);

	handler_install(SIGUSR1);
	zassert_ok(sigignore(SIGUSR1));
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, 0);
	zassert_equal(sigset(SIGUSR1, SIG_DFL), SIG_IGN);
}

ZTEST(xsi_signals, test_siginterrupt)
{
	errno = 0;
	zassert_equal(siginterrupt(-1, 1), -1);
	zassert_equal(errno, EINVAL);

	handler_install(SIGUSR1);
	zassert_ok(siginterrupt(SIGUSR1, 1));
	zassert_ok(siginterrupt(SIGUSR1, 0));
	/* the disposition stays */
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, SIGUSR1);
	zassert_equal(sigset(SIGUSR1, SIG_DFL), handler);
}

static void sigpause_raiser(struct k_work *work)
{
	ARG_UNUSED(work);

	(void)killpg(0, SIGUSR1);
}

ZTEST(xsi_signals, test_sigpause)
{
	static K_WORK_DELAYABLE_DEFINE(raiser, sigpause_raiser);

	errno = 0;
	zassert_equal(sigpause(-1), -1);
	zassert_equal(errno, EINVAL);

	handler_install(SIGUSR1);
	zassert_ok(sighold(SIGUSR1));
	k_work_schedule(&raiser, K_MSEC(10));
	errno = 0;
	zassert_equal(sigpause(SIGUSR1), -1);
	zassert_equal(errno, EINTR);
	zassert_equal(handled, SIGUSR1);

	/* the mask is restored: the signal is held again */
	handled = 0;
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, 0, "the mask was not restored");
	zassert_ok(sigrelse(SIGUSR1));
	zassert_equal(handled, SIGUSR1);
	disposition_reset(SIGUSR1);
}

ZTEST(xsi_signals, test_sigrelse)
{
	errno = 0;
	zassert_equal(sigrelse(-1), -1);
	zassert_equal(errno, EINVAL);

	handler_install(SIGUSR2);
	zassert_ok(sighold(SIGUSR2));
	zassert_ok(sigrelse(SIGUSR2));
	zassert_ok(killpg(0, SIGUSR2));
	zassert_equal(handled, SIGUSR2);
	/* releasing a signal that is not held is not an error */
	zassert_ok(sigrelse(SIGUSR2));
	disposition_reset(SIGUSR2);
}

ZTEST(xsi_signals, test_sigset)
{
	zassert_equal(sigset(-1, SIG_DFL), SIG_ERR);

	handled = 0;
	zassert_equal(sigset(SIGUSR1, handler), SIG_DFL);
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, SIGUSR1);

	/* SIG_HOLD adds to the mask and leaves the disposition */
	handled = 0;
	zassert_equal(sigset(SIGUSR1, SIG_HOLD), handler);
	zassert_ok(killpg(0, SIGUSR1));
	zassert_equal(handled, 0);

	/* from a held signal: SIG_HOLD back, and the signal is released */
	zassert_equal(sigset(SIGUSR1, handler), SIG_HOLD);
	zassert_equal(handled, SIGUSR1);

	zassert_equal(sigset(SIGUSR1, SIG_DFL), handler);
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	/* the test thread takes the signals the tests generate, whatever it was started with */
	(void)sigrelse(SIGUSR1);
	(void)sigrelse(SIGUSR2);
	disposition_reset(SIGUSR1);
	disposition_reset(SIGUSR2);
}

ZTEST_SUITE(xsi_signals, NULL, NULL, before, NULL, NULL);

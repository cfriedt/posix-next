/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"

static void timer_create_default_evp(void)
{
	siginfo_t info = {0};

	/* NULL sigevent: as if SIGEV_SIGNAL, SIGALRM, with the timer ID as the value */
	zassert_ok(timer_create(CLOCK_MONOTONIC, NULL, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, 0);

	zassert_equal(accept_sig(SIGALRM, &info, 10 * PERIOD_MS), SIGALRM,
		      "default-sigevent timer did not deliver SIGALRM");
	zassert_equal(info.si_code, SI_TIMER, "si_code is %d, not SI_TIMER", info.si_code);
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host's default value is a kernel timer id, not the timer_t bits */
		zassert_equal_ptr(info.si_value.sival_ptr, (void *)(uintptr_t)timerid,
				  "default notification value is not the timer ID");
	}
}

static void timer_create_sigev_signal(void)
{
	siginfo_t info = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, PERIOD_MS);

	/* each accepted expiry signal permits the next to be queued (one-pending model) */
	for (int i = 0; i < 3; ++i) {
		zassert_equal(accept_sig(TEST_SIGNAL_VAL, &info, 10 * PERIOD_MS),
			      TEST_SIGNAL_VAL, "periodic expiry signal %d not delivered", i);
		zassert_equal(info.si_value.sival_int, TEST_SIGNAL_VAL);
		zassert_equal(info.si_code, SI_TIMER);
	}
}

static void sig_handler(int signo, siginfo_t *info, void *ctx)
{
	ARG_UNUSED(ctx);

	++exp_count;
	zassert_equal(signo, TEST_SIGNAL_VAL);
	zassert_equal(info->si_value.sival_int, TEST_SIGNAL_VAL);
}

static void timer_create_sigev_signal_handler(void)
{
	struct sigaction act = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = sig_handler,
	};
	struct sigaction oact = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	exp_count = 0;
	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(TEST_SIGNAL_VAL, &act, &oact));
	set_sig_blocked(TEST_SIGNAL_VAL, false);

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, PERIOD_MS);

	/* delivery happens in this thread's context; sleep in short slices to accept it */
	for (int i = 0; (exp_count < 2) && (i < 100); ++i) {
		test_sleep_ms(PERIOD_MS / 4);
	}
	zassert_true(exp_count >= 2, "handler ran %d times", exp_count);

	zassert_ok(timer_delete(timerid));
	timerid = INVALID_TIMERID;
	set_sig_blocked(TEST_SIGNAL_VAL, true);
	zassert_ok(sigaction(TEST_SIGNAL_VAL, &oact, NULL));
}

static void thread_fn(union sigval val)
{
	zassert_equal(val.sival_int, TEST_SIGNAL_VAL);
	fn_overrun = timer_getoverrun(timerid);
	++exp_count;
}

static void timer_create_sigev_thread(void)
{
	struct sigevent sig = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_notify_function = thread_fn,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	exp_count = 0;
	fn_overrun = -1;

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, PERIOD_MS);

	test_sleep_ms(5 * PERIOD_MS + PERIOD_MS / 2);

	/* the helper accepts each expiry promptly: no coalescing while it keeps up */
	zassert_true(exp_count >= 3, "notification function ran %d times", exp_count);
	/* timer_getoverrun() inside the notification function is valid and non-negative */
	zassert_true(fn_overrun >= 0, "overrun inside notification function: %d", fn_overrun);

	/* deleting with callbacks potentially in flight must be safe */
	zassert_ok(timer_delete(timerid));
	timerid = INVALID_TIMERID;
}

#if defined(SIGEV_THREAD_ID) && !defined(CONFIG_NATIVE_LIBC)

/* the helper runs in another thread, which cannot reach this one's stack under userspace */
static ZTEST_BMEM int received_signal;

static void *tid_helper_fn(void *arg)
{
	siginfo_t info = {0};
	int *const out = arg;

	set_sig_blocked(TEST_SIGNAL_VAL, true);
	*out = accept_sig(TEST_SIGNAL_VAL, &info, 20 * PERIOD_MS);

	return NULL;
}

static void timer_create_sigev_thread_id(void)
{
	pthread_t th;
	struct sigevent sig = {
		.sigev_notify = SIGEV_THREAD_ID,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	received_signal = -1;
	zassert_ok(pthread_create(&th, NULL, tid_helper_fn, &received_signal));
	/* let the helper block its signal and enter sigtimedwait() */
	test_sleep_ms(PERIOD_MS / 2);

	sig.sigev_notify_thread_id = (pid_t)th;
	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, 0);

	zassert_ok(pthread_join(th, NULL));
	zassert_equal(received_signal, TEST_SIGNAL_VAL,
		      "targeted thread did not receive the signal");
}
#endif /* defined(SIGEV_THREAD_ID) && !defined(CONFIG_NATIVE_LIBC) */

static void timer_create_signal_less(void)
{
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
	};

	/* without signal-based expiry notification, only SIGEV_NONE is supported */
	zassert_equal(timer_create(CLOCK_MONOTONIC, &sig, &timerid), -1);
	zassert_equal(errno, ENOTSUP);
	timerid = INVALID_TIMERID;

	sig.sigev_notify = SIGEV_NONE;
	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, 0);
	test_sleep_ms(2 * PERIOD_MS);
	zassert_equal(timer_getoverrun(timerid), 0);
}

static void timer_create_errors(void)
{
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	zassert_equal(timer_create((clockid_t)4242, &sig, &timerid), -1);
	zassert_equal(errno, EINVAL);
	timerid = INVALID_TIMERID;
}

ZTEST_USER(posix_timers, test_timer_create)
{
	timer_create_errors();
	section_reset();

	if (!IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		/* without signal-based expiry notification, only SIGEV_NONE is supported */
		timer_create_signal_less();
		return;
	}

	timer_create_default_evp();
	section_reset();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host cannot block process-directed signals across simulator threads */
		timer_create_sigev_signal();
		section_reset();
	}

	timer_create_sigev_signal_handler();
	section_reset();

	timer_create_sigev_thread();
	section_reset();

#if defined(SIGEV_THREAD_ID) && !defined(CONFIG_NATIVE_LIBC)
	/* on the host, sigev_notify_thread_id requires gettid(); covered by Linux */
	timer_create_sigev_thread_id();
	section_reset();
#endif
}

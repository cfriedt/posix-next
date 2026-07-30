/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "timers_tests.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(timer_test);

ZTEST_DMEM timer_t timerid = INVALID_TIMERID;
ZTEST_BMEM volatile int exp_count;
ZTEST_BMEM volatile int fn_overrun;

#ifdef CONFIG_NATIVE_LIBC
/*
 * On the host, timer signals are process-directed and may be delivered to any native
 * simulator service thread, where the default action would kill the process; and a blocking
 * host sigtimedwait() would stall the whole simulator. Instead, a process-wide handler
 * captures deliveries and accept_sig() consumes them from a counter.
 */
static volatile int lc_count;
static int lc_taken;
static siginfo_t lc_info;

static void lc_capture(int signo, siginfo_t *info, void *ctx)
{
	ARG_UNUSED(signo);
	ARG_UNUSED(ctx);

	lc_info = *info;
	++lc_count;
}

void lc_install(int signo)
{
	struct sigaction act = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = lc_capture,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(signo, &act, NULL));
}
#endif /* CONFIG_NATIVE_LIBC */

/* accept one instance of (blocked) signo, waiting up to timeout_ms */
int accept_sig(int signo, siginfo_t *info, int timeout_ms)
{
	sigset_t set;
	struct timespec timeout = {
		.tv_sec = timeout_ms / MSEC_PER_SEC,
		.tv_nsec = (timeout_ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
	};

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));

#ifdef CONFIG_NATIVE_LIBC
	/*
	 * Consume deliveries against a taken-counter rather than a per-call baseline: an
	 * immediately-expiring timer (e.g. a past TIMER_ABSTIME deadline) can run the handler
	 * on the arming call's syscall-exit path, i.e. before this function is entered, and a
	 * baseline snapshot would wait forever for a count change that already happened.
	 * Like sigtimedwait(), an already-pending delivery satisfies the call.
	 */
	for (int elapsed = 0;; elapsed += 5) {
		if (lc_count != lc_taken) {
			++lc_taken;
			if (info != NULL) {
				*info = lc_info;
			}
			return lc_info.si_signo;
		}
		if (elapsed >= timeout_ms) {
			break;
		}
		test_sleep_ms(5);
	}
	errno = EAGAIN;
	return -1;
#endif /* CONFIG_NATIVE_LIBC */

	return sigtimedwait(&set, info, &timeout);
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

#ifdef CONFIG_NATIVE_LIBC
	lc_install(TEST_SIGNAL_VAL);
	lc_install(SIGALRM);
#else
	if (IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		/* sigtimedwait()-style acceptance requires the signals to be blocked */
		set_sig_blocked(TEST_SIGNAL_VAL, true);
		set_sig_blocked(SIGALRM, true);
	}
#endif
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	/* delete leftovers even when an assertion aborted the test mid-way */
	if (timerid != INVALID_TIMERID) {
		(void)timer_delete(timerid);
		timerid = INVALID_TIMERID;
	}

	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* let an in-flight delivery from the deleted timer land before consuming */
		test_sleep_ms(5);
		drain_sig(TEST_SIGNAL_VAL);
	} else if (IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		drain_sig(TEST_SIGNAL_VAL);
		drain_sig(SIGALRM);
	}

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* let detached notification threads recycle their stacks */
		k_msleep(2 * CONFIG_SYS_THREAD_RECYCLER_DELAY_MS);
	}
}

ZTEST_SUITE(posix_timers, NULL, NULL, before, after, NULL);

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

#ifdef CONFIG_NATIVE_LIBC
/*
 * Under the host libc the notification signal is a real process-directed
 * signal: leaving it blocked and waiting for it with sigtimedwait() would
 * stall native_sim, and leaving it unhandled would kill the process. Capture
 * deliveries in a handler instead and consume them from a counter.
 */
static volatile int lc_count;
static siginfo_t lc_info;

static void lc_capture(int signo, siginfo_t *info, void *ctx)
{
	ARG_UNUSED(ctx);

	lc_info = *info;
	lc_info.si_signo = signo;
	lc_count++;
}

void mq_arm_sig(int signo)
{
	struct sigaction act = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = lc_capture,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(signo, &act, NULL));
}
#else
void mq_arm_sig(int signo)
{
	sigset_t set;

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
}
#endif /* CONFIG_NATIVE_LIBC */

int mq_accept_sig(int signo, siginfo_t *info, int timeout_ms)
{
#ifdef CONFIG_NATIVE_LIBC
	const int start = lc_count;

	for (int elapsed = 0; elapsed <= timeout_ms; elapsed += 5) {
		if (lc_count != start) {
			if (info != NULL) {
				*info = lc_info;
			}
			return lc_info.si_signo;
		}
		usleep(USEC_PER_MSEC * 5);
	}

	errno = EAGAIN;
	return -1;
#else
	sigset_t set;
	struct timespec timeout = {
		.tv_sec = timeout_ms / (long)MSEC_PER_SEC,
		.tv_nsec = (timeout_ms % (long)MSEC_PER_SEC) * (long)NSEC_PER_MSEC,
	};

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));

	return sigtimedwait(&set, info, &timeout);
#endif /* CONFIG_NATIVE_LIBC */
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	mq_test_section_reset();
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	mq_test_section_reset();
}

ZTEST_SUITE(posix_message_passing, NULL, NULL, before, after, NULL);

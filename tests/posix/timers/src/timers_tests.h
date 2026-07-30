/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTS_POSIX_TIMERS_SRC_TIMERS_TESTS_H_
#define TESTS_POSIX_TIMERS_SRC_TIMERS_TESTS_H_

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

#define INVALID_TIMERID ((timer_t)-1)

#define PERIOD_MS       100
#define PERIOD_NS       (PERIOD_MS * NSEC_PER_MSEC)
#define TEST_SIGNAL_VAL SIGUSR1

/* the timer under test; deleted by the suite's after() hook when a test aborts mid-way */
extern timer_t timerid;
extern volatile int exp_count;
extern volatile int fn_overrun;

/* accept one instance of (blocked) signo, waiting up to timeout_ms */
int accept_sig(int signo, siginfo_t *info, int timeout_ms);

#ifdef CONFIG_NATIVE_LIBC
void lc_install(int signo);
#endif

/* Block or unblock a signal for the calling thread. */
static inline void set_sig_blocked(int signo, bool block)
{
	sigset_t set;

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));
	zassert_ok(pthread_sigmask(block ? SIG_BLOCK : SIG_UNBLOCK, &set, NULL));
}

static inline void drain_sig(int signo)
{
	while (accept_sig(signo, NULL, 0) >= 0) {
	}
}

static inline void test_sleep_ms(int ms)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/*
		 * host libc timers fire in real time, which simulated time may lag, and
		 * signal delivery interrupts the sleep
		 */
		struct timespec rem = {
			.tv_sec = ms / MSEC_PER_SEC,
			.tv_nsec = (ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
		};

		while ((nanosleep(&rem, &rem) == -1) && (errno == EINTR)) {
		}
	} else {
		k_sleep(K_MSEC(ms));
	}
}

static inline void arm_ms(timer_t id, int flags, int64_t value_ms, int64_t interval_ms)
{
	struct itimerspec value = {
		.it_value.tv_sec = value_ms / MSEC_PER_SEC,
		.it_value.tv_nsec = (value_ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
		.it_interval.tv_sec = interval_ms / MSEC_PER_SEC,
		.it_interval.tv_nsec = (interval_ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
	};

	zassert_ok(timer_settime(id, flags, &value, NULL));
}

/* the per-test cleanup, applied between the sections of a merged per-unit test */
static inline void section_reset(void)
{
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

#endif /* TESTS_POSIX_TIMERS_SRC_TIMERS_TESTS_H_ */

/*
 * Copyright (c) 2018 Friedt Professional Engineering Services, Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/clock.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#include "../../shared/nanosleep_common.h"

static void nanosleep_errors_errno(void)
{
	common_errors(SELECT_NANOSLEEP, CLOCK_REALTIME, 0);
}

static void nanosleep_execution(void)
{
	/* sleep for 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 0, 1);

	/* sleep for 1us + 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 0, 1001);

	/* sleep for 500000000ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 0, 500000000);

	/* sleep for 1s */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 1, 0);

	/* sleep for 1s + 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 1, 1);

	/* sleep for 1s + 1us + 1ns */
	common_lower_bound_check(SELECT_NANOSLEEP, 0, 0, 1, 1001);
}

static void nanosleep_signal_handler(int sig)
{
	ARG_UNUSED(sig);
}

static void nanosleep_interrupted(void)
{
	struct sigaction act = {.sa_handler = nanosleep_signal_handler};
	struct sigevent sev = {.sigev_notify = SIGEV_SIGNAL, .sigev_signo = SIGUSR1};
	struct itimerspec its = {.it_value = {.tv_nsec = 100 * NSEC_PER_MSEC}};
	struct timespec req = {.tv_sec = 10};
	struct timespec rem = {0};
	sigset_t set;
	timer_t tid;

	/* a caught signal ends the sleep with EINTR and the time left */
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGUSR1));
	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));
	zassert_ok(sigaction(SIGUSR1, &act, NULL));
	zassert_ok(timer_create(CLOCK_MONOTONIC, &sev, &tid));
	zassert_ok(timer_settime(tid, 0, &its, NULL));

	zassert_equal(nanosleep(&req, &rem), -1);
	zassert_equal(errno, EINTR);
	zassert_true(rem.tv_sec >= 9, "%lld s left", (long long)rem.tv_sec);

	zassert_ok(timer_delete(tid));
	act.sa_handler = SIG_DFL;
	zassert_ok(sigaction(SIGUSR1, &act, NULL));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
}

ZTEST(posix_timers, test_nanosleep)
{
	nanosleep_errors_errno();
	nanosleep_execution();
	/* the host libc delivers a process-directed signal to any of the simulator's host threads */
	if (IS_ENABLED(CONFIG_TIMER_SIGNAL) && !IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		nanosleep_interrupted();
	}
}


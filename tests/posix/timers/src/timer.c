/*
 * Copyright (c) 2018 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>

#define SECS_TO_SLEEP  2
#define DURATION_SECS  1
#define DURATION_NSECS 0
#define PERIOD_SECS    0
#define PERIOD_NSECS   100000000

#define TEST_SIGNAL_VAL SIGRTMIN

LOG_MODULE_REGISTER(timer_test);

static int exp_count;
static timer_t timerid = -1;

void handler(union sigval val)
{
	++exp_count;
	LOG_DBG("Handler Signal value %d for %d times", val.sival_int, exp_count);
	zassert_equal(val.sival_int, TEST_SIGNAL_VAL);
}

static void sig_handler(int signo, siginfo_t *info, void *ctx)
{
	ARG_UNUSED(ctx);

	++exp_count;
	LOG_DBG("Signal %d delivered %d times", signo, exp_count);
	zassert_equal(signo, TEST_SIGNAL_VAL);
	zassert_equal(info->si_value.sival_int, TEST_SIGNAL_VAL);
}

static void install_sig_handler(void)
{
	struct k_sig_set mask;
	struct sigaction act = {
		.sa_flags = SA_SIGINFO,
		.sa_sigaction = sig_handler,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(TEST_SIGNAL_VAL, &act, NULL));

	/*
	 * Zephyr kernel threads block all signals by default, and some libc
	 * sigset_t types are too small for realtime signal numbers; unblock
	 * through the kernel API, as the realtime_signals suite does.
	 */
	zassert_ok(k_sig_emptyset(&mask));
	zassert_ok(k_sig_addset(&mask, TEST_SIGNAL_VAL));
	zassert_ok(k_sig_mask(K_SIG_UNBLOCK, &mask, NULL));
}

void test_timer(clockid_t clock_id, int sigev_notify)
{
	struct sigevent sig = {0};
	struct itimerspec value, ovalue;
	struct timespec ts, te;
	int64_t nsecs_elapsed, secs_elapsed;

	exp_count = 0;
	sig.sigev_notify = sigev_notify;
	sig.sigev_value.sival_int = TEST_SIGNAL_VAL;
	if (sigev_notify == SIGEV_SIGNAL) {
		sig.sigev_signo = TEST_SIGNAL_VAL;
		install_sig_handler();
	} else {
		sig.sigev_notify_function = handler;
	}

	/*TESTPOINT: Check if timer is created successfully*/
	zassert_ok(timer_create(clock_id, &sig, &timerid));

	value.it_value.tv_sec = DURATION_SECS;
	value.it_value.tv_nsec = DURATION_NSECS;
	value.it_interval.tv_sec = PERIOD_SECS;
	value.it_interval.tv_nsec = PERIOD_NSECS;
	zassert_ok(timer_settime(timerid, 0, &value, &ovalue));
	usleep(100 * USEC_PER_MSEC);
	/*TESTPOINT: Check if timer has started successfully*/
	zassert_ok(timer_gettime(timerid, &value));

	LOG_DBG("Timer fires every %d secs and  %d nsecs", (int)value.it_interval.tv_sec,
		(int)value.it_interval.tv_nsec);
	LOG_DBG("Time remaining to fire %d secs and  %d nsecs", (int)value.it_value.tv_sec,
		(int)value.it_value.tv_nsec);

	zassert_ok(clock_gettime(clock_id, &ts));
	k_sleep(K_SECONDS(SECS_TO_SLEEP));
	zassert_ok(clock_gettime(clock_id, &te));

	if (te.tv_nsec >= ts.tv_nsec) {
		secs_elapsed = te.tv_sec - ts.tv_sec;
		nsecs_elapsed = te.tv_nsec - ts.tv_nsec;
	} else {
		nsecs_elapsed = NSEC_PER_SEC + te.tv_nsec - ts.tv_nsec;
		secs_elapsed = (te.tv_sec - ts.tv_sec - 1);
	}

	uint64_t elapsed = secs_elapsed * NSEC_PER_SEC + nsecs_elapsed;
	uint64_t first_sig = value.it_value.tv_sec * NSEC_PER_SEC + value.it_value.tv_nsec;
	uint64_t sig_interval = value.it_interval.tv_sec * NSEC_PER_SEC + value.it_interval.tv_nsec;
	int expected_signal_count = (elapsed - first_sig) / sig_interval + 1;

	/*TESTPOINT: Check if POSIX timer test passed*/
	zassert_within(exp_count, expected_signal_count, 1, "POSIX timer test has failed %i != %i",
		       exp_count, expected_signal_count);
}

ZTEST(posix_timers, test_CLOCK_REALTIME__SIGEV_SIGNAL)
{
	test_timer(CLOCK_REALTIME, SIGEV_SIGNAL);
}

ZTEST(posix_timers, test_CLOCK_REALTIME__SIGEV_THREAD)
{
	test_timer(CLOCK_REALTIME, SIGEV_THREAD);
}

ZTEST(posix_timers, test_CLOCK_MONOTONIC__SIGEV_SIGNAL)
{
#if defined(_POSIX_MONOTONIC_CLOCK)
	test_timer(CLOCK_MONOTONIC, SIGEV_SIGNAL);
#else
	ztest_test_skip();
#endif
}

ZTEST(posix_timers, test_CLOCK_MONOTONIC__SIGEV_THREAD)
{
#if defined(_POSIX_MONOTONIC_CLOCK)
	test_timer(CLOCK_MONOTONIC, SIGEV_THREAD);
#else
	ztest_test_skip();
#endif
}

ZTEST(posix_timers, test_timer_overrun)
{
	struct sigevent sig = {0};
	struct itimerspec value;

	sig.sigev_notify = SIGEV_NONE;

	zassert_ok(timer_create(CLOCK_REALTIME, &sig, &timerid));

	/*Set the timer to expire every 500 milliseconds*/
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 500000000;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 500000000;
	zassert_ok(timer_settime(timerid, 0, &value, NULL));
	k_sleep(K_MSEC(2500));

	zassert_equal(timer_getoverrun(timerid), 4, "Number of overruns is incorrect");
}

ZTEST(posix_timers, test_one_shot__SIGEV_SIGNAL)
{
	struct sigevent sig = {0};
	struct itimerspec value;

	exp_count = 0;
	sig.sigev_notify = SIGEV_SIGNAL;
	sig.sigev_signo = TEST_SIGNAL_VAL;
	sig.sigev_value.sival_int = TEST_SIGNAL_VAL;
	install_sig_handler();

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));

	/*Set the timer to expire only once*/
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;
	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 100 * NSEC_PER_MSEC;
	zassert_ok(timer_settime(timerid, 0, &value, NULL));
	k_sleep(K_MSEC(300));

	zassert_equal(exp_count, 1, "Number of expiry is incorrect");
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	if (timerid != -1) {
		(void)timer_delete(timerid);
		timerid = -1;
		/* Give cancelled SIGEV_THREAD worker time to be recycled */
		k_sleep(K_MSEC(2 * CONFIG_SYS_THREAD_RECYCLER_DELAY_MS));
	}
	/* queued signals have drained through the still-installed handler */
	(void)signal(TEST_SIGNAL_VAL, SIG_DFL);
}

ZTEST_SUITE(posix_timers, NULL, NULL, NULL, after, NULL);

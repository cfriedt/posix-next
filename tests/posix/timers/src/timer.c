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

#define TEST_SIGNAL_VAL 34 /* normally SIGRTMIN */

LOG_MODULE_REGISTER(timer_test);

static int exp_count;
static timer_t timerid = -1;

void handler(union sigval val)
{
	++exp_count;
	LOG_DBG("Handler Signal value %d for %d times", val.sival_int, exp_count);
	zassert_equal(val.sival_int, TEST_SIGNAL_VAL);
}

void test_timer(clockid_t clock_id, int sigev_notify)
{
	struct sigevent sig = {0};
	struct itimerspec value, ovalue;
	struct timespec ts, te;
	int64_t nsecs_elapsed, secs_elapsed;

	exp_count = 0;
	sig.sigev_notify = sigev_notify;
	sig.sigev_notify_function = handler;
	sig.sigev_value.sival_int = TEST_SIGNAL_VAL;

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
	sig.sigev_notify_function = handler;
	sig.sigev_value.sival_int = TEST_SIGNAL_VAL;

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

ZTEST(posix_timers, test_TIMER_ABSTIME_large_timeout)
{
	struct sigevent sig = {0};
	struct itimerspec value = {0};
	struct timespec orig;
	const struct timespec ref = {.tv_sec = 1700000000};

	exp_count = 0;
	sig.sigev_notify = SIGEV_THREAD;
	sig.sigev_notify_function = handler;
	sig.sigev_value.sival_int = 20;

	zassert_ok(timer_create(CLOCK_REALTIME, &sig, &timerid));

	/*
	 * A wall clock of 1.7e9 s scaled to nanoseconds overflows 32-bit time_t arithmetic,
	 * and a multi-second offset keeps the overflow from cancelling in the subtraction of
	 * deadline and now: an unwidened conversion fires at once or truncates the wait.
	 */
	zassert_ok(clock_gettime(CLOCK_REALTIME, &orig));
	zassert_ok(clock_settime(CLOCK_REALTIME, &ref));
	value.it_value.tv_sec = ref.tv_sec + 5;
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &value, NULL));

	k_sleep(K_MSEC(2000));
	zassert_equal(exp_count, 0, "5 s absolute deadline fired within 2 s");
	k_sleep(K_MSEC(3500));
	zassert_equal(exp_count, 1, "5 s absolute deadline fired %d times, expected 1", exp_count);

	zassert_ok(clock_settime(CLOCK_REALTIME, &orig));
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	if (timerid != -1) {
		(void)timer_delete(timerid);
		timerid = -1;
		/* Give cancelled SIGEV_THREAD worker time to be recycled */
		k_msleep(10);
	}
}

ZTEST_SUITE(posix_timers, NULL, NULL, NULL, after, NULL);

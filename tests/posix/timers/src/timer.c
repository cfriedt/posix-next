/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>


#define INVALID_TIMERID ((timer_t)-1)

#define PERIOD_MS       100
#define PERIOD_NS       (PERIOD_MS * NSEC_PER_MSEC)
#define TEST_SIGNAL_VAL SIGUSR1

LOG_MODULE_REGISTER(timer_test);

static ZTEST_DMEM timer_t timerid = INVALID_TIMERID;
static ZTEST_BMEM volatile int exp_count;
static ZTEST_BMEM volatile int fn_overrun;

/* Block or unblock a signal for the calling thread. */
static void set_sig_blocked(int signo, bool block)
{
	sigset_t set;

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));
	zassert_ok(pthread_sigmask(block ? SIG_BLOCK : SIG_UNBLOCK, &set, NULL));
}

static void test_sleep_ms(int ms);

#ifdef CONFIG_NATIVE_LIBC
/*
 * On the host, timer signals are process-directed and may be delivered to any native
 * simulator service thread, where the default action would kill the process; and a blocking
 * host sigtimedwait() would stall the whole simulator. Instead, a process-wide handler
 * captures deliveries and accept_sig() consumes them from a counter.
 */
static volatile int lc_count;
static siginfo_t lc_info;

static void lc_capture(int signo, siginfo_t *info, void *ctx)
{
	ARG_UNUSED(signo);
	ARG_UNUSED(ctx);

	lc_info = *info;
	++lc_count;
}

static void lc_install(int signo)
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
static int accept_sig(int signo, siginfo_t *info, int timeout_ms)
{
	sigset_t set;
	struct timespec timeout = {
		.tv_sec = timeout_ms / MSEC_PER_SEC,
		.tv_nsec = (timeout_ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
	};

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));

#ifdef CONFIG_NATIVE_LIBC
	{
		const int start = lc_count;

		for (int elapsed = 0; elapsed <= timeout_ms; elapsed += 5) {
			if (lc_count != start) {
				if (info != NULL) {
					*info = lc_info;
				}
				return lc_info.si_signo;
			}
			test_sleep_ms(5);
		}
		errno = EAGAIN;
		return -1;
	}
#endif /* CONFIG_NATIVE_LIBC */

	return sigtimedwait(&set, info, &timeout);
}

static void drain_sig(int signo)
{
	while (accept_sig(signo, NULL, 0) >= 0) {
	}
}

static void test_sleep_ms(int ms)
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

static void arm_ms(timer_t id, int flags, int64_t value_ms, int64_t interval_ms)
{
	struct itimerspec value = {
		.it_value.tv_sec = value_ms / MSEC_PER_SEC,
		.it_value.tv_nsec = (value_ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
		.it_interval.tv_sec = interval_ms / MSEC_PER_SEC,
		.it_interval.tv_nsec = (interval_ms % MSEC_PER_SEC) * NSEC_PER_MSEC,
	};

	zassert_ok(timer_settime(id, flags, &value, NULL));
}

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

ZTEST_USER(posix_timers, test_timer_getoverrun)
{
	if (!IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		/* signal-based expiry notification requires the kernel signal subsystem */
		ztest_test_skip();
	}
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host cannot block process-directed signals across simulator threads */
		ztest_test_skip();
	}
	int overrun;
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, PERIOD_MS, PERIOD_MS);

	/* let ~5 periods elapse with the (blocked) signal left pending */
	test_sleep_ms(5 * PERIOD_MS + PERIOD_MS / 2);

	/* exactly one instance is queued; accepting it latches the overrun */
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 0), TEST_SIGNAL_VAL);
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 0), -1,
		      "more than one signal instance queued for one timer");

	overrun = timer_getoverrun(timerid);
	zassert_true((overrun >= 2) && (overrun <= 6), "overrun %d outside expected window",
		     overrun);
	/* non-destructive: repeated reads return the latched value */
	zassert_equal(timer_getoverrun(timerid), overrun);
	zassert_equal(timer_getoverrun(timerid), overrun);
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

static void timer_gettime_armed_and_expired(void)
{
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	arm_ms(timerid, 0, 2 * PERIOD_MS, 0);

	zassert_ok(timer_gettime(timerid, &its));
	zassert_true((its.it_value.tv_sec > 0) || (its.it_value.tv_nsec > 0),
		     "armed timer reports disarmed");

	test_sleep_ms(3 * PERIOD_MS);

	/* an expired one-shot reports a zero it_value */
	zassert_ok(timer_gettime(timerid, &its));
	zassert_true((its.it_value.tv_sec == 0) && (its.it_value.tv_nsec == 0),
		     "expired one-shot still reports armed");
	zassert_equal(timer_getoverrun(timerid), 0);
}

static void timer_settime_abstime_monotonic(void)
{
	struct timespec now;
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));

	/* future absolute deadline */
	zassert_ok(clock_gettime(CLOCK_MONOTONIC, &now));
	its.it_value = now;
	its.it_value.tv_nsec += 2 * PERIOD_NS;
	while (its.it_value.tv_nsec >= (long)NSEC_PER_SEC) {
		its.it_value.tv_nsec -= (long)NSEC_PER_SEC;
		its.it_value.tv_sec++;
	}
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &its, NULL));
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 10 * PERIOD_MS), TEST_SIGNAL_VAL,
		      "future absolute deadline did not fire");

	/*
	 * An absolute deadline in the past expires immediately. The accept
	 * window is an upper bound, not a delay: it matches the file's other
	 * probes so that slow, loaded runners (e.g. -O0 sanitizer binaries
	 * under linux_compat real-time slowdown) do not flake on delivery
	 * latency.
	 */
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 1;
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &its, NULL));
	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 10 * PERIOD_MS), TEST_SIGNAL_VAL,
		      "past absolute deadline did not fire immediately");
}

static void timer_settime_abstime_realtime(void)
{
	struct timespec now;
	struct timespec jump;
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = TEST_SIGNAL_VAL,
		.sigev_value.sival_int = TEST_SIGNAL_VAL,
	};

	zassert_ok(timer_create(CLOCK_REALTIME, &sig, &timerid));

	/* absolute wall-clock deadline one hour out */
	zassert_ok(clock_gettime(CLOCK_REALTIME, &now));
	its.it_value = now;
	its.it_value.tv_sec += 3600;
	zassert_ok(timer_settime(timerid, TIMER_ABSTIME, &its, NULL));

	/* jumping the wall clock past the deadline must fire the timer promptly */
	jump = its.it_value;
	jump.tv_sec += 1;
	zassert_ok(clock_settime(CLOCK_REALTIME, &jump));

	zassert_equal(accept_sig(TEST_SIGNAL_VAL, NULL, 10 * PERIOD_MS), TEST_SIGNAL_VAL,
		      "clock_settime() past the deadline did not fire the timer");

	/* restore the wall clock roughly */
	zassert_ok(clock_settime(CLOCK_REALTIME, &now));
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

static void timer_settime_errors(void)
{
	struct itimerspec its = {0};
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));

	its.it_value.tv_nsec = NSEC_PER_SEC; /* invalid */
	zassert_equal(timer_settime(timerid, 0, &its, NULL), -1);
	zassert_equal(errno, EINVAL);

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* NULL-argument handling is unspecified; the host may fault instead */
		zassert_equal(timer_settime(timerid, 0, NULL, NULL), -1);
		zassert_equal(errno, EINVAL);
	}
}

static void timer_gettime_errors(void)
{
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));

	zassert_equal(timer_gettime(timerid, NULL), -1);
	zassert_equal(errno, EINVAL);
}

ZTEST_USER(posix_timers, test_timer_delete)
{
	struct sigevent sig = {
		.sigev_notify = SIGEV_NONE,
	};

	if (IS_ENABLED(CONFIG_NATIVE_LIBC) || IS_ENABLED(CONFIG_USERSPACE)) {
		/* host libc semantics differ; in user mode a stale handle faults instead */
		ztest_test_skip();
	}

	zassert_ok(timer_create(CLOCK_MONOTONIC, &sig, &timerid));
	zassert_ok(timer_delete(timerid));

	/* double delete must fail cleanly */
	zassert_equal(timer_delete(timerid), -1);
	zassert_equal(errno, EINVAL);
	timerid = INVALID_TIMERID;
}

/* the per-test cleanup, applied between the sections of a merged per-function test */
static void section_reset(void)
{
	if (timerid != INVALID_TIMERID) {
		(void)timer_delete(timerid);
		timerid = INVALID_TIMERID;
	}

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		drain_sig(TEST_SIGNAL_VAL);
		drain_sig(SIGALRM);
	}

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* let detached helper threads recycle their stacks */
		k_msleep(2 * CONFIG_SYS_THREAD_RECYCLER_DELAY_MS);
	}
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

ZTEST_USER(posix_timers, test_timer_settime)
{
	timer_settime_errors();
	section_reset();

	if (!IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		/* expiry observation below is signal-based */
		return;
	}

	timer_settime_abstime_monotonic();
	section_reset();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* the host wall clock cannot be jumped from a test */
		timer_settime_abstime_realtime();
		section_reset();
	}
}

ZTEST_USER(posix_timers, test_timer_gettime)
{
	timer_gettime_armed_and_expired();
	section_reset();

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* NULL-argument handling is unspecified; the host may fault instead */
		timer_gettime_errors();
		section_reset();
	}
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

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && IS_ENABLED(CONFIG_TIMER_SIGNAL)) {
		drain_sig(TEST_SIGNAL_VAL);
		drain_sig(SIGALRM);
	}

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* let detached notification threads recycle their stacks */
		k_msleep(2 * CONFIG_SYS_THREAD_RECYCLER_DELAY_MS);
	}
}


ZTEST_SUITE(posix_timers, NULL, NULL, before, after, NULL);

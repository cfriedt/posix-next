/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <limits.h>

#ifndef _POSIX_REALTIME_SIGNALS
#define _POSIX_REALTIME_SIGNALS 200809L
#endif
#include <signal.h>
#include <zephyr/posix/sys/select.h>
#include <pthread.h>
#include <string.h>

#if !defined(CONFIG_NATIVE_LIBC)
#undef sigemptyset
#undef sigfillset
#undef sigaddset
#undef sigdelset
#undef sigismember
#endif

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#ifdef CONFIG_USERSPACE
int pthread_kill(pthread_t thread, int sig);
#endif

#define SIGSET_NLONGS (sizeof(sigset_t) / sizeof(unsigned long))

BUILD_ASSERT(SIGSET_NLONGS > 0, "sigset_t has no storage");

#ifndef RTSIG_MAX
#define RTSIG_MAX CONFIG_POSIX_RTSIG_MAX
#endif
#ifndef SIGQUEUE_MAX
#define SIGQUEUE_MAX _POSIX_SIGQUEUE_MAX
#endif

BUILD_ASSERT(RTSIG_MAX > 0);
BUILD_ASSERT(SIGQUEUE_MAX > 0);
BUILD_ASSERT(SIGQUEUE_MAX >= SIGNAL_QUEUE_SIZE);

static ZTEST_BMEM bool rt_sigset_usable;

static void skip_if_rt_sigset_too_small(void)
{
	if (!rt_sigset_usable) {
		/*
		 * Some libc's provide a sigset_t that is too small for real-time
		 * signals at SIGRTMIN (e.g. picolibc on 32-bit native_sim).
		 */
		ztest_test_skip();
	}
}

static void block_rt_signals(void);

static ZTEST_BMEM sigset_t rt_sigset;

static ZTEST_BMEM struct sigqueue_work {
	struct k_work_delayable dwork;
	pthread_t target;
} sigq_work;

static void do_queue(struct k_work *work)
{
	struct sigqueue_work *sq_work = CONTAINER_OF(
		CONTAINER_OF(work, struct k_work_delayable, work), struct sigqueue_work, dwork);

	zassert_ok(sigqueue((pid_t)sq_work->target, SIGRTMIN, (union sigval){0}));
}

static void queue_signal_after_ms(pthread_t target, int delay_ms)
{
	(void)k_work_cancel_delayable(&sigq_work.dwork);
	sigq_work.target = target;
	k_work_init_delayable(&sigq_work.dwork, do_queue);
	k_work_schedule(&sigq_work.dwork, K_MSEC(delay_ms));
}

static void drain_pending_rt_signals(void)
{
	sigset_t set = rt_sigset;
	struct timespec timeout = {0};
	int signo;

	while ((signo = sigtimedwait(&set, NULL, &timeout)) >= 0) {
		(void)signo;
	}
	errno = 0;
}

static void unblock_rt_signal(int signo)
{
	struct k_sig_set mask;

	if (k_is_user_context()) {
		sigset_t pmask;

		sigemptyset(&pmask);
		zassert_ok(sigaddset(&pmask, signo));
		zassert_ok(pthread_sigmask(SIG_UNBLOCK, &pmask, NULL));
		return;
	}

	zassert_ok(k_sig_emptyset(&mask));
	zassert_ok(k_sig_addset(&mask, signo));
	zassert_ok(k_sig_mask(K_SIG_UNBLOCK, &mask, NULL));
}

static void block_rt_signals(void)
{
	sigset_t mask;

	sigfillset(&mask);
	for (int i = 0; i < RTSIG_MAX; ++i) {
		zassert_ok(sigdelset(&mask, (SIGRTMIN + i)));
	}

	zassert_ok(pthread_sigmask(SIG_BLOCK, &mask, NULL));
}

static void test_sigqueue(void)
{
	sigset_t set = rt_sigset;
	siginfo_t info;
	struct timespec timeout = {0};

	{
		zassert_not_ok(sigqueue(-1, -1, (union sigval){0}));
#if defined(CONFIG_NATIVE_LIBC)
		zassert_true(errno == EINVAL || errno == ESRCH,
			     "errno was %d, expected EINVAL or ESRCH", errno);
#else
		zassert_equal(errno, EINVAL);
#endif

		zassert_not_ok(sigqueue(-1, SIGUSR1, (union sigval){0}));
		zassert_equal(errno, ESRCH, "errno was %d instead of ESRCH (%d)", errno, ESRCH);

		zassert_not_ok(sigqueue((pid_t)pthread_self(), -1, (union sigval){0}));
#if defined(CONFIG_NATIVE_LIBC)
		zassert_true(errno == EINVAL || errno == ESRCH,
			     "errno was %d, expected EINVAL or ESRCH", errno);
#else
		zassert_equal(errno, EINVAL, "errno was %d instead of EINVAL (%d)", errno, EINVAL);
#endif
	}

	block_rt_signals();

	for (int i = 0; i < SIGNAL_QUEUE_SIZE; ++i) {
		zassert_ok(
			sigqueue((pid_t)pthread_self(), SIGRTMIN, (union sigval){.sival_int = i}),
			"failed to queue the %d-th signal", i);
	}

	zassert_not_ok(sigqueue((pid_t)pthread_self(), SIGRTMIN, (union sigval){0}));
	zassert_equal(errno, EAGAIN);

	for (int i = 0; i < SIGNAL_QUEUE_SIZE; ++i) {
		int actual;

		info = (siginfo_t){0};
		actual = sigtimedwait(&set, &info, &timeout);

		zassert_equal(
			SIGRTMIN, actual,
			"iteration %d expected SIGRTMIN (%d) but sigtimedwait() returned %d "
			"(errno: %d)",
			i, SIGRTMIN, actual, errno);
		zassert_equal(info.si_value.sival_int, i);
	}

	for (int i = RTSIG_MAX - 1; i >= 0; --i) {
		zassert_ok(sigqueue((pid_t)pthread_self(), (SIGRTMIN + i), (union sigval){0}),
			   "unable to queue signal %d", (SIGRTMIN + i));
	}

	for (int i = 0; i < RTSIG_MAX; ++i) {
		int actual = sigtimedwait(&set, NULL, &timeout);

		zassert_equal((SIGRTMIN + i), actual,
			      "expected signal %d, but sigtimedwait() returned %d", (SIGRTMIN + i),
			      actual);
	}
}

static void test_sigtimedwait(void)
{
	sigset_t set = rt_sigset;
	siginfo_t info;
	pid_t self = (pid_t)pthread_self();
	uint32_t begin_ms, delta_ms, end_ms;
	struct timespec timeout = {0};
	struct timespec wait_100ms = {
		.tv_sec = 0,
		.tv_nsec = 100 * NSEC_PER_MSEC,
	};
	struct timespec wait_300ms = {
		.tv_sec = 0,
		.tv_nsec = 300 * NSEC_PER_MSEC,
	};

	const struct stw_args_exp {
		const sigset_t *set;
		siginfo_t *info;
		struct timespec *timeout;
		int expected_errno;
	} harness[] = {
		{NULL, NULL, NULL, EINVAL},
		{NULL, NULL, &timeout, EINVAL},
		{NULL, &info, NULL, EINVAL},
		{NULL, &info, &timeout, EINVAL},
		{&set, NULL, NULL, 0},
		{&set, NULL, &timeout, 0},
		{&set, &info, NULL, 0},
		{&set, &info, &timeout, 0},
	};

	block_rt_signals();

	ARRAY_FOR_EACH_PTR(harness, a) {
		errno = 0;
		if (a->expected_errno == 0) {
			zassert_ok(sigqueue(self, SIGRTMIN, (union sigval){0}));
			zassert_equal(SIGRTMIN, sigtimedwait(a->set, a->info, a->timeout));
			if (a->info != NULL) {
				zassert_equal(a->info->si_signo, SIGRTMIN);
			}
		} else {
			zassert_equal(-1, sigtimedwait(a->set, a->info, a->timeout));
			zassert_equal(errno, a->expected_errno);
		}
	}

	begin_ms = k_uptime_get_32();
	zassert_equal(-1, sigtimedwait(&set, NULL, &timeout));
	zassert_equal(errno, EAGAIN);
	end_ms = k_uptime_get_32();
	delta_ms = end_ms - begin_ms;
	zassert_true(delta_ms < 50);

	begin_ms = k_uptime_get_32();
	zassert_equal(-1, sigtimedwait(&set, NULL, &wait_100ms));
	zassert_equal(errno, EAGAIN);
	end_ms = k_uptime_get_32();
	delta_ms = end_ms - begin_ms;
	zassert_true(delta_ms >= 100);

	begin_ms = k_uptime_get_32();
	queue_signal_after_ms(pthread_self(), 100);
	zassert_equal(SIGRTMIN, sigtimedwait(&set, NULL, &wait_300ms));
	end_ms = k_uptime_get_32();
	delta_ms = end_ms - begin_ms;
	zassert_true(delta_ms >= 100);
	zassert_true(delta_ms < 200);
}

static void test_sigwaitinfo(void)
{
	sigset_t set = rt_sigset;
	siginfo_t info;
	uint32_t begin_ms, delta_ms, end_ms;

	const struct swi_args_exp {
		const sigset_t *set;
		siginfo_t *info;
		int expected_errno;
	} harness[] = {
		{NULL, NULL, EINVAL},
		{NULL, &info, EINVAL},
		{&set, NULL, 0},
		{&set, &info, 0},
	};

	block_rt_signals();

	ARRAY_FOR_EACH_PTR(harness, a) {
		errno = 0;
		if (a->expected_errno == 0) {
			zassert_ok(sigqueue((pid_t)pthread_self(), SIGRTMIN,
					    (union sigval){.sival_int = 42}));
			zassert_equal(SIGRTMIN, sigwaitinfo(a->set, a->info));
			if (a->info != NULL) {
				zassert_equal(a->info->si_signo, SIGRTMIN);
				zassert_equal(a->info->si_value.sival_int, 42);
			}
		} else {
			zassert_equal(-1, sigwaitinfo(a->set, a->info));
			zassert_equal(errno, a->expected_errno);
		}
	}

	begin_ms = k_uptime_get_32();
	queue_signal_after_ms(pthread_self(), 100);
	zassert_equal(SIGRTMIN, sigwaitinfo(&set, NULL));
	end_ms = k_uptime_get_32();
	delta_ms = end_ms - begin_ms;
	zassert_true(delta_ms >= 100);
	zassert_true(delta_ms < 200);
}

static ZTEST_BMEM volatile bool rt_handler_called;

static void rt_k_handler(int sig, struct k_sig_info *info, void *ucontext)
{
	ARG_UNUSED(sig);
	ARG_UNUSED(info);
	ARG_UNUSED(ucontext);

	rt_handler_called = true;
}

/*
 * Zephyr delivers queued signals from z_sig_handle() at thread swap points, not
 * k_sig_queue() syscall wrapper. That is not Linux-style preemption; signals
 * from other threads are delivered when the target thread next completes a
 * syscall.
 */
static void test_rt_delivery_deferred(void)
{
	struct k_sig_action action = {
		.handler = rt_k_handler,
	};

	rt_handler_called = false;
	unblock_rt_signal(K_SIG_RTMIN);
	zassert_ok(k_sig_action(K_SIG_RTMIN, &action, NULL));
	zassert_ok(k_sig_queue((k_pid_t)k_current_get(), K_SIG_RTMIN, (union k_sig_val){0}));
	zassert_true(rt_handler_called,
		     "handler runs by the time k_sig_queue() returns (syscall check)");

	action.handler = K_SIG_DFL;
	zassert_ok(k_sig_action(K_SIG_RTMIN, &action, NULL));
}

ZTEST(posix_realtime_signals, test_sigqueue)
{
	skip_if_rt_sigset_too_small();
	test_sigqueue();
}

ZTEST(posix_realtime_signals, test_sigtimedwait)
{
	skip_if_rt_sigset_too_small();
	test_sigtimedwait();
}

ZTEST(posix_realtime_signals, test_sigwaitinfo)
{
	skip_if_rt_sigset_too_small();
	test_sigwaitinfo();
}

ZTEST(posix_realtime_signals, test_rt_delivery_deferred)
{
	test_rt_delivery_deferred();
}

ZTEST_USER(posix_realtime_signals, test_sigqueue_user)
{
	skip_if_rt_sigset_too_small();
	test_sigqueue();
}

ZTEST_USER(posix_realtime_signals, test_sigtimedwait_user)
{
	skip_if_rt_sigset_too_small();
	test_sigtimedwait();
}

ZTEST_USER(posix_realtime_signals, test_sigwaitinfo_user)
{
	skip_if_rt_sigset_too_small();
	test_sigwaitinfo();
}

#ifdef CONFIG_USERSPACE
ZTEST_USER(posix_realtime_signals, test_cannot_sigqueue_kernel_from_user)
{
	extern struct k_thread z_main_thread;

	zassert_equal(EPERM, pthread_kill((pthread_t)(uintptr_t)&z_main_thread, SIGRTMIN));
}
#endif

static void *setup(void)
{
	sigset_t probe;

	sigemptyset(&probe);
	if (sigaddset(&probe, SIGRTMIN) != 0) {
		/*
		 * Some libc's provide a sigset_t that is too small for real-time
		 * signals at SIGRTMIN, reserve that signal number,
		 */
		rt_sigset_usable = false;
		return NULL;
	}

	rt_sigset_usable = true;
	sigemptyset(&rt_sigset);
	for (int i = 0; i < RTSIG_MAX; ++i) {
		const int signo = SIGRTMIN + i;

		if ((signo - 1) >= SIGSET_NLONGS * BITS_PER_LONG) {
			break;
		}
		if (sigaddset(&rt_sigset, signo) != 0) {
			break;
		}
	}

	return NULL;
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	(void)k_work_cancel_delayable(&sigq_work.dwork);
	drain_pending_rt_signals();
	{
		struct k_sig_action action = {
			.handler = K_SIG_DFL,
		};

		(void)k_sig_action(K_SIG_RTMIN, &action, NULL);
	}
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	(void)k_work_cancel_delayable(&sigq_work.dwork);
}

ZTEST_SUITE(posix_realtime_signals, NULL, setup, before, after, NULL);

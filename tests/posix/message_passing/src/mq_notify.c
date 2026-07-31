/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

#include <pthread.h>
#include <signal.h>

#define NOTIFY_SIGNO  SIGUSR1
#define NOTIFY_SIGVAL 4242

static ZTEST_BMEM volatile bool notify_fn_ran;
static ZTEST_BMEM volatile int notify_fn_value;

static void notify_fn(union sigval value)
{
	notify_fn_value = value.sival_int;
	notify_fn_ran = true;
}

static ZTEST_BMEM int notify_si_code;

static int notify_sig_accept(int ms)
{
	siginfo_t info = {0};

	if (mq_accept_sig(NOTIFY_SIGNO, &info, ms) < 0) {
		return -1;
	}

	notify_si_code = info.si_code;

	return info.si_value.sival_int;
}

static int notify_sig_code(void)
{
	return notify_si_code;
}

static void mq_notify_sigev_none(void)
{
	struct sigevent sev = {
		.sigev_notify = SIGEV_NONE,
	};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	zassert_ok(mq_notify(mqd, &sev));
	zassert_ok(mq_send(mqd, "quiet", 6, 0));

	/* SIGEV_NONE runs nothing, but the arrival still consumes the
	 * registration, so a second one is accepted
	 */
	zassert_ok(mq_notify(mqd, &sev));
	zassert_ok(mq_notify(mqd, NULL));

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_notify_sigev_signal(void)
{
	char buf[MQ_MSG_SIZE];
	struct sigevent sev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = NOTIFY_SIGNO,
		.sigev_value.sival_int = NOTIFY_SIGVAL,
	};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	mq_drain_sig(NOTIFY_SIGNO);

	zassert_ok(mq_notify(mqd, &sev));
	zassert_ok(mq_send(mqd, "signal", 7, 0));

	zassert_equal(notify_sig_accept(MQ_TIMEOUT_MS), NOTIFY_SIGVAL,
		      "notification signal not delivered");
	zassert_equal(notify_sig_code(), SI_MESGQ);

	/* the registration was consumed: a further arrival does not signal */
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), 7);
	zassert_ok(mq_send(mqd, "again", 6, 0));
	zassert_equal(notify_sig_accept(MQ_TIMEOUT_MS), -1,
		      "notification fired after being consumed");

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_notify_only_on_empty_to_non_empty(void)
{
	char buf[MQ_MSG_SIZE];
	struct sigevent sev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = NOTIFY_SIGNO,
		.sigev_value.sival_int = NOTIFY_SIGVAL,
	};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	mq_drain_sig(NOTIFY_SIGNO);

	/* arming while the queue is non-empty does not fire on further sends */
	zassert_ok(mq_send(mqd, "first", 6, 0));
	zassert_ok(mq_notify(mqd, &sev));
	zassert_ok(mq_send(mqd, "second", 7, 0));
	zassert_equal(notify_sig_accept(MQ_TIMEOUT_MS), -1,
		      "notification fired on a non-empty queue");

	/* draining and refilling crosses the transition, which does fire */
	zassert_true(mq_receive(mqd, buf, sizeof(buf), NULL) > 0);
	zassert_true(mq_receive(mqd, buf, sizeof(buf), NULL) > 0);
	zassert_ok(mq_send(mqd, "third", 6, 0));
	zassert_equal(notify_sig_accept(MQ_TIMEOUT_MS), NOTIFY_SIGVAL);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_notify_sigev_thread(void)
{
	struct sigevent sev = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_notify_function = notify_fn,
		.sigev_value.sival_int = NOTIFY_SIGVAL,
	};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	notify_fn_ran = false;
	notify_fn_value = 0;

	zassert_ok(mq_notify(mqd, &sev));
	zassert_ok(mq_send(mqd, "thread", 7, 0));

	for (int i = 0; (i < 100) && !notify_fn_ran; i++) {
		usleep(USEC_PER_MSEC * 10);
	}

	zassert_true(notify_fn_ran, "notification function did not run");
	zassert_equal(notify_fn_value, NOTIFY_SIGVAL);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

#if defined(_POSIX_THREAD_ATTR_STACKSIZE) && defined(_POSIX_THREAD_PRIORITY_SCHEDULING) &&        \
	!defined(CONFIG_NATIVE_LIBC)

#define NOTIFY_ATTR_STACK_SIZE 6144

static ZTEST_BMEM volatile int notify_fn_zprio;
static ZTEST_BMEM volatile size_t notify_fn_stack;

/* record the notification thread's kernel priority and (supervisor only) stack size */
static void attr_probe_fn(union sigval val)
{
	ARG_UNUSED(val);

	notify_fn_zprio = k_thread_priority_get(k_current_get());
	if (!k_is_user_context()) {
		notify_fn_stack = k_current_get()->stack_info.size;
	}
	notify_fn_ran = true;
}

static void attr_probe_await(mqd_t mqd, const char *msg)
{
	char buf[MQ_MSG_SIZE];

	notify_fn_ran = false;
	zassert_ok(mq_send(mqd, msg, strlen(msg) + 1, 0));
	for (int i = 0; (i < 100) && !notify_fn_ran; i++) {
		usleep(USEC_PER_MSEC * 10);
	}
	zassert_true(notify_fn_ran, "notification function did not run");

	/* drain, so the next arrival crosses the empty -> non-empty transition */
	zassert_true(mq_receive(mqd, buf, sizeof(buf), NULL) >= 0);
}

static void mq_notify_sigev_thread_attributes(void)
{
	pthread_attr_t attr;
	struct sched_param param = {
		/* the minimum SCHED_RR priority; sched_get_priority_min() is another group */
		.sched_priority = 0,
	};
	struct sigevent sev = {
		.sigev_notify = SIGEV_THREAD,
		.sigev_notify_function = attr_probe_fn,
	};
	int default_prio;
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* without attributes, the notification thread inherits the registrant's priority */
	zassert_ok(mq_notify(mqd, &sev));
	attr_probe_await(mqd, "no attributes");
	default_prio = notify_fn_zprio;

	/*
	 * FIXME(oversized-mapped-user-stacks): dynamically-allocated user
	 * stacks larger than CONFIG_SYS_THREAD_STACK_SIZE are broken
	 * (k_object_map_size() and the sys_thread recycler mishandle them),
	 * so from user mode request only the guaranteed slot size.
	 */
	const size_t req_stack =
		k_is_user_context() ? CONFIG_SYS_THREAD_STACK_SIZE : NOTIFY_ATTR_STACK_SIZE;

	zassert_ok(pthread_attr_init(&attr));
	zassert_ok(pthread_attr_setstacksize(&attr, req_stack));
	zassert_ok(pthread_attr_setschedpolicy(&attr, SCHED_RR));
	zassert_ok(pthread_attr_setschedparam(&attr, &param));
	sev.sigev_notify_attributes = &attr;

	zassert_ok(mq_notify(mqd, &sev));
	/* translated at registration time: the attribute object may be destroyed at once */
	zassert_ok(pthread_attr_destroy(&attr));

	attr_probe_await(mqd, "with attributes");
	zassert_not_equal(notify_fn_zprio, default_prio,
			  "sigev_notify_attributes scheduling parameters were not honored");
	if (!k_is_user_context()) {
		zassert_true(notify_fn_stack >= NOTIFY_ATTR_STACK_SIZE,
			     "requested %d stack, notification thread has %zu",
			     NOTIFY_ATTR_STACK_SIZE, notify_fn_stack);
	}

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}
#endif /* _POSIX_THREAD_ATTR_STACKSIZE && _POSIX_THREAD_PRIORITY_SCHEDULING && !NATIVE_LIBC */

#if defined(SIGEV_THREAD_ID) && !defined(CONFIG_NATIVE_LIBC)
/* the helper runs in another thread, which cannot reach the registrant's stack under userspace */
static ZTEST_BMEM int received_sigval;

static void *tid_helper_fn(void *arg)
{
	int *const out = arg;
	sigset_t set;
	siginfo_t info = {0};
	struct timespec ts = {
		.tv_nsec = MQ_TIMEOUT_MS * NSEC_PER_MSEC,
	};

	(void)sigemptyset(&set);
	(void)sigaddset(&set, NOTIFY_SIGNO);
	(void)pthread_sigmask(SIG_BLOCK, &set, NULL);

	if ((sigtimedwait(&set, &info, &ts) == NOTIFY_SIGNO) && (info.si_code == SI_MESGQ)) {
		*out = info.si_value.sival_int;
	}

	return NULL;
}

static void mq_notify_sigev_thread_id(void)
{
	pthread_t th;
	struct sigevent sev = {
		.sigev_notify = SIGEV_THREAD_ID,
		.sigev_signo = NOTIFY_SIGNO,
		.sigev_value.sival_int = NOTIFY_SIGVAL,
	};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	received_sigval = -1;
	zassert_ok(pthread_create(&th, NULL, tid_helper_fn, &received_sigval));
	/* let the helper block its signal and enter sigtimedwait() */
	usleep(USEC_PER_MSEC * 50);

	/* the arrival signal goes to the named thread, not the registrant */
	sev.sigev_notify_thread_id = (pid_t)th;
	zassert_ok(mq_notify(mqd, &sev));
	zassert_ok(mq_send(mqd, "targeted", 9, 0));

	zassert_ok(pthread_join(th, NULL));
	zassert_equal(received_sigval, NOTIFY_SIGVAL, "targeted thread did not receive the signal");

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}
#endif /* defined(SIGEV_THREAD_ID) && !defined(CONFIG_NATIVE_LIBC) */

static void mq_notify_removal(void)
{
	struct sigevent sev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = NOTIFY_SIGNO,
		.sigev_value.sival_int = NOTIFY_SIGVAL,
	};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	mq_drain_sig(NOTIFY_SIGNO);

	zassert_ok(mq_notify(mqd, &sev));
	zassert_ok(mq_notify(mqd, NULL));

	/* a removed registration does not fire */
	zassert_ok(mq_send(mqd, "removed", 8, 0));
	zassert_equal(notify_sig_accept(MQ_TIMEOUT_MS), -1,
		      "notification fired after removal");

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_notify_errors(void)
{
	struct sigevent sev = {
		.sigev_notify = SIGEV_SIGNAL,
		.sigev_signo = NOTIFY_SIGNO,
		.sigev_value.sival_int = NOTIFY_SIGVAL,
	};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* removing when nothing is armed is not an error */
	zassert_ok(mq_notify(mqd, NULL));

	/* only one registration at a time */
	zassert_ok(mq_notify(mqd, &sev));
	zassert_equal(mq_notify(mqd, &sev), -1);
	zassert_equal(errno, EBUSY);
	zassert_ok(mq_notify(mqd, NULL));

	/* an unknown notification type */
	sev.sigev_notify = 4242;
	zassert_equal(mq_notify(mqd, &sev), -1);
	zassert_equal(errno, EINVAL);

	/* a descriptor that is not open */
	zassert_equal(mq_notify((mqd_t)-1, NULL), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

ZTEST_USER(posix_message_passing, test_mq_notify)
{
	mq_arm_sig(NOTIFY_SIGNO);

	mq_notify_sigev_none();
	mq_test_section_reset();
	mq_notify_errors();
	mq_test_section_reset();

	if (IS_ENABLED(CONFIG_POSIX_SIGNALS)) {
		mq_notify_sigev_signal();
		mq_test_section_reset();
		mq_notify_only_on_empty_to_non_empty();
		mq_test_section_reset();
		mq_notify_removal();
		mq_test_section_reset();
	}

#if defined(SIGEV_THREAD_ID) && !defined(CONFIG_NATIVE_LIBC)
	/* on the host, sigev_notify_thread_id requires gettid(); covered by Linux */
	if (IS_ENABLED(CONFIG_POSIX_SIGNALS) && IS_ENABLED(CONFIG_POSIX_THREADS)) {
		mq_notify_sigev_thread_id();
		mq_test_section_reset();
	}
#endif /* defined(SIGEV_THREAD_ID) && !defined(CONFIG_NATIVE_LIBC) */

	/* SIGEV_THREAD is dispatched by the kernel: no pthreads or supervisor mode needed */
	if (IS_ENABLED(CONFIG_NATIVE_LIBC) ||
	    (IS_ENABLED(CONFIG_SIGNAL) && IS_ENABLED(CONFIG_SYS_THREAD) &&
	     IS_ENABLED(CONFIG_THREAD_DETACH))) {
		mq_notify_sigev_thread();
		mq_test_section_reset();

#if defined(_POSIX_THREAD_ATTR_STACKSIZE) && defined(_POSIX_THREAD_PRIORITY_SCHEDULING) &&        \
	!defined(CONFIG_NATIVE_LIBC)
		mq_notify_sigev_thread_attributes();
		mq_test_section_reset();
#endif
	}
}

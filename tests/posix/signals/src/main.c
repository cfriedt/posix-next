/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "_main.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#define BYTES_PER_LONG (sizeof(unsigned long))

#define SIGNO_WORD_IDX(_signo) ((_signo - 1) / BITS_PER_LONG)
#define SIGNO_WORD_BIT(_signo) ((_signo - 1) & BIT_MASK(LOG2(BITS_PER_LONG)))

#if defined(CONFIG_NATIVE_LIBC)
#ifdef __GLIBC__
/* glibc internally aliases sigset_t to something significantly smaller than the external
 * definition. This is apparent by simply calling sigemptyset() and checking the values of the
 * sigset_t in terms of unsigned longs. Even though it's publicly defined as capable of holding
 * 1024 bits (i.e. is 8 64-bit words, or 16 32-bit words). The actual result of sigemptyset() is
 * that only the first 64-bits are cleared, corresponding to the definition of SIGSET_NLONGS below.
 */
#define SIGSET_NLONGS (_NSIG / (BITS_PER_LONG))
#else
#error "Unknown native libc"
#endif
#else
#define SIGSET_NLONGS (sizeof(sigset_t) / sizeof(unsigned long))
#endif
#define SIGSET_SIZE (SIGSET_NLONGS * BYTES_PER_LONG)

BUILD_ASSERT(SIGSET_NLONGS > 0, "sigset_t has no storage");

/*
 * Some C libraries provide a sigset_t that is too small to hold the real-time signals Zephyr is
 * configured for. Cases that reach into individual bits skip the ones that do not fit.
 */
#define SIGNO_FITS(_signo) (((_signo) - 1) < (int)(SIGSET_NLONGS * BITS_PER_LONG))

ZTEST_BMEM struct test_signals_record test_signals_state;

void test_signals_record_reset(void)
{
	test_signals_state = (struct test_signals_record){0};
}

void test_signals_handler(int signo)
{
	test_signals_state.calls++;
	test_signals_state.signo = signo;
	test_signals_state.mask_valid = sigprocmask(SIG_SETMASK, NULL, &test_signals_state.mask) == 0;
}

void test_signals_sigaction_handler(int signo, siginfo_t *info, void *context)
{
	ARG_UNUSED(context);

	test_signals_handler(signo);
	test_signals_state.si_signo = (info == NULL) ? -1 : info->si_signo;
	test_signals_state.si_code = (info == NULL) ? -1 : info->si_code;
}

bool test_signals_blocked(int signo)
{
	sigset_t mask;

	zassert_ok(sigprocmask(SIG_SETMASK, NULL, &mask));

	return sigismember(&mask, signo) == 1;
}

void test_signals_reset(int signo)
{
	sigset_t set;
	struct sigaction act = {
		.sa_handler = SIG_IGN,
		.sa_flags = 0,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, signo));

	/*
	 * Ignore the signal before unblocking it, so that an instance left pending by an earlier
	 * case is discarded rather than acted on. Unblocking first would run the default action,
	 * which on a hosted C library ends the whole test binary.
	 */
	zassert_ok(sigaction(signo, &act, NULL));
	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));

	/*
	 * The signal is left ignored rather than default. A signal directed at the process can be
	 * taken by any thread at any time, so leaving SIG_DFL installed between cases would let a
	 * late instance run the default action, and blocking it here would not help. Cases that
	 * need the default disposition use a signal that nothing generates.
	 */
}

void test_signals_set_default(int signo)
{
	struct sigaction act = {
		.sa_handler = SIG_DFL,
		.sa_flags = 0,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(signo, &act, NULL));
}

bool test_signals_wait_for_delivery(int calls)
{
	/*
	 * A signal directed at the process rather than at a thread is delivered to whichever
	 * thread is willing to take it, and not necessarily before the generating call returns.
	 */
	for (int i = 0; (i < 200) && (test_signals_state.calls < calls); i++) {
		k_msleep(10);
	}

	return test_signals_state.calls >= calls;
}

void test_signals_install(int signo, void *handler, int flags)
{
	struct sigaction act = {
		.sa_handler = (void (*)(int))handler,
		.sa_flags = flags,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(signo, &act, NULL));

	test_signals_record_reset();
}

ZTEST_USER(posix_signals, test_sigemptyset)
{
	unsigned long _set[SIGSET_NLONGS];
	sigset_t *const set = (sigset_t *)_set;

	ARRAY_FOR_EACH(_set, i) {
		_set[i] = -1;
	}

	zassert_ok(sigemptyset(set));

	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], 0u, "set.sig[%d] is not empty: 0x%lx", i, _set[i]);
	}

	/* an empty set has no members */
	zassert_equal(0, sigismember(set, SIGUSR1));
	zassert_equal(0, sigismember(set, SIGTERM));
}

ZTEST_USER(posix_signals, test_sigfillset)
{
	sigset_t set = (sigset_t){0};

	zassert_ok(sigfillset(&set));

	/* every signal the application may use is a member */
	zassert_equal(1, sigismember(&set, SIGUSR1));
	zassert_equal(1, sigismember(&set, SIGTERM));
	zassert_equal(1, sigismember(&set, SIGKILL));
	zassert_equal(1, sigismember(&set, SIGSTOP));

	/*
	 * The two signals reserved for thread cancellation are not, and cannot be added back.
	 * Zephyr follows the Linux NPTL implementation here.
	 */
	zassert_equal(-1, sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER + 1));
	zassert_equal(EINVAL, errno);
}

ZTEST_USER(posix_signals, test_sigaddset)
{
	int signo;
	sigset_t set = (sigset_t){0};
	unsigned long *const _set = (unsigned long *)&set;
	sigset_t target = (sigset_t){0};
	unsigned long *const _target = (unsigned long *)&target;

	/* signal numbers outside the supported range are rejected */
	zassert_equal(-1, sigaddset(&set, -1));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaddset(&set, 0));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaddset(&set, SIGRTMAX + 1));
	zassert_equal(EINVAL, errno);

	/* as are the two signals reserved for thread cancellation */
	zassert_equal(-1, sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER + 1));
	zassert_equal(EINVAL, errno);

	/* a rejected signal number leaves the set untouched */
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], 0u, "a rejected signal number modified the set");
	}

	signo = SIGHUP;
	zassert_ok(sigaddset(&set, signo));
	WRITE_BIT(_target[SIGNO_WORD_IDX(signo)], SIGNO_WORD_BIT(signo), 1);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	/* adding a second signal does not disturb the first */
	signo = SIGSYS;
	zassert_ok(sigaddset(&set, signo));
	WRITE_BIT(_target[SIGNO_WORD_IDX(signo)], SIGNO_WORD_BIT(signo), 1);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	/* adding a signal that is already a member is not an error */
	zassert_ok(sigaddset(&set, SIGSYS));
	zassert_equal(1, sigismember(&set, SIGSYS));

	if (SIGNO_FITS(SIGRTMIN)) {
		zassert_ok(sigaddset(&set, SIGRTMIN));
		zassert_equal(1, sigismember(&set, SIGRTMIN));
	}
}

ZTEST_USER(posix_signals, test_sigdelset)
{
	int signo;
	sigset_t set = (sigset_t){0};
	unsigned long *const _set = (unsigned long *)&set;
	sigset_t target = (sigset_t){0};
	unsigned long *const _target = (unsigned long *)&target;

	zassert_equal(-1, sigdelset(&set, -1));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigdelset(&set, 0));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigdelset(&set, SIGRTMAX + 1));
	zassert_equal(EINVAL, errno);

	zassert_ok(sigfillset(&set));
	zassert_ok(sigfillset(&target));

	signo = SIGHUP;
	zassert_ok(sigdelset(&set, signo));
	WRITE_BIT(_target[SIGNO_WORD_IDX(signo)], SIGNO_WORD_BIT(signo), 0);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	signo = SIGSYS;
	zassert_ok(sigdelset(&set, signo));
	WRITE_BIT(_target[SIGNO_WORD_IDX(signo)], SIGNO_WORD_BIT(signo), 0);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	/* removing a signal that is not a member is not an error */
	zassert_ok(sigdelset(&set, SIGSYS));
	zassert_equal(0, sigismember(&set, SIGSYS));
}

ZTEST_USER(posix_signals, test_sigismember)
{
	sigset_t set = (sigset_t){0};

	zassert_equal(-1, sigismember(&set, -1));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigismember(&set, 0));
	zassert_equal(EINVAL, errno);
	zassert_equal(-1, sigismember(&set, SIGRTMAX + 1));
	zassert_equal(EINVAL, errno);

	zassert_ok(sigaddset(&set, SIGHUP));
	zassert_ok(sigaddset(&set, SIGSYS));

	zassert_equal(1, sigismember(&set, SIGHUP), "%s expected to be member", "SIGHUP");
	zassert_equal(1, sigismember(&set, SIGSYS), "%s expected to be member", "SIGSYS");

	zassert_equal(0, sigismember(&set, SIGKILL), "%s not expected to be member", "SIGKILL");
	zassert_equal(0, sigismember(&set, SIGTERM), "%s not expected to be member", "SIGTERM");
}

ZTEST_USER(posix_signals, test_pthread_sigmask)
{
	test_pthread_sigmask_common(pthread_sigmask);
}

/*
 * The shared expectations are written against pthread_sigmask(), which returns an error number.
 * sigprocmask() reports the same errors through errno, so translate before handing it over.
 */
static int sigprocmask_as_pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
	if (sigprocmask(how, set, oset) == 0) {
		return 0;
	}

	return errno;
}

ZTEST_USER(posix_signals, test_sigprocmask)
{
	sigset_t set;
	sigset_t oset;

	/*
	 * POSIX leaves sigprocmask() unspecified in a multi-threaded process, but Zephyr has no
	 * processes: the calling thread is the whole process, so it behaves like pthread_sigmask()
	 * and differs only in reporting errors through errno rather than the return value.
	 */
	test_pthread_sigmask_common(sigprocmask_as_pthread_sigmask);

	zassert_ok(sigemptyset(&set));
	zassert_equal(-1, sigprocmask(-1, &set, NULL), "an invalid 'how' was accepted");
	zassert_equal(EINVAL, errno);

	/*
	 * Asking to block SIGKILL or SIGSTOP is not an error. Zephyr only enforces the restriction
	 * for user threads, which are the ones running application code; a supervisor thread is
	 * not a POSIX thread and is trusted to mask whatever it likes.
	 */
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGKILL));
	zassert_ok(sigaddset(&set, SIGSTOP));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));

	if (IS_ENABLED(CONFIG_USERSPACE) && k_is_user_context()) {
		zassert_ok(sigprocmask(SIG_SETMASK, NULL, &oset));
		zassert_equal(0, sigismember(&oset, SIGKILL), "%s must not be blockable", "SIGKILL");
		zassert_equal(0, sigismember(&oset, SIGSTOP), "%s must not be blockable", "SIGSTOP");
	}

	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));
}

static void *setup(void)
{
	test_pthread_sigmask_init_expectation_mask();
	return NULL;
}

ZTEST_SUITE(posix_signals, NULL, setup, NULL, NULL, NULL);

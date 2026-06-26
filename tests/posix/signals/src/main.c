/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

void test_pthread_sigmask_init_expectation_mask(void);
void test_pthread_sigmask_common(int (*sigmask)(int how, const sigset_t *set, sigset_t *oset));

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

ZTEST(posix_signals, test_sigemptyset)
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
}

ZTEST(posix_signals, test_sigfillset)
{
	sigset_t set = (sigset_t){0};

	zassert_ok(sigfillset(&set));
	zassert_equal(-1, sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER));
	zassert_equal(errno, EINVAL);
	zassert_equal(-1, sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER + 1));
	zassert_equal(errno, EINVAL);
	zassert_equal(1, sigismember(&set, SIGUSR1));
}

ZTEST(posix_signals, test_sigaddset_oor)
{
	sigset_t set = (sigset_t){0};

	zassert_equal(sigaddset(&set, -1), -1, "rc should be -1");
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");

	zassert_equal(sigaddset(&set, 0), -1, "rc should be -1");
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");

	zassert_equal(sigaddset(&set, SIGRTMAX + 1), -1, "rc should be -1");
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");
}

ZTEST(posix_signals, test_sigaddset)
{
	int signo;
	sigset_t set = (sigset_t){0};
	unsigned long *const _set = (unsigned long *)&set;
	sigset_t target = (sigset_t){0};
	unsigned long *const _target = (unsigned long *)&target;

	signo = SIGHUP;
	zassert_ok(sigaddset(&set, signo));
	WRITE_BIT(_target[0], signo - 1, 1);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	signo = SIGSYS;
	zassert_ok(sigaddset(&set, signo));
	WRITE_BIT(_target[0], signo - 1, 1);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	/* TODO: move rt signal tests to realtime_signals testsuite */
	const int rtsigs[] = {SIGRTMIN, SIGRTMAX};

	ARRAY_FOR_EACH(rtsigs, i) {
		int expected_ret = 0;
		int expected_errno = 0;

		signo = rtsigs[i];
		if ((signo - 1) >= SIGSET_NLONGS * BITS_PER_LONG) {
			/* Some libc's provide a sigset_t that is too small for real-time signals */
			expected_ret = -1;
			expected_errno = EINVAL;
		} else {
			WRITE_BIT(_target[(signo - 1) / BITS_PER_LONG], (signo - 1) % BITS_PER_LONG,
				  1);
		}

		errno = 0;
		zassert_equal(sigaddset(&set, signo), expected_ret);
		zassert_equal(errno, expected_errno);
		for (int i = 0; i < SIGSET_NLONGS; i++) {
			zassert_equal(_set[i], _target[i],
				      "set.sig[%d of %d] has content: %lx, expected %lx", i,
				      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
		}
	}
}

ZTEST(posix_signals, test_sigdelset_oor)
{
	sigset_t set = (sigset_t){0};

	zassert_equal(sigdelset(&set, -1), -1, "rc should be -1");
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");

	zassert_equal(sigdelset(&set, 0), -1, "rc should be -1");
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");

	zassert_equal(sigdelset(&set, SIGRTMAX + 1), -1, "rc should be -1");
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");
}

ZTEST(posix_signals, test_sigdelset)
{
	int signo;
	sigset_t set = (sigset_t){0};
	unsigned long *const _set = (unsigned long *)&set;
	sigset_t target = (sigset_t){0};
	unsigned long *const _target = (unsigned long *)&target;

	zassert_ok(sigfillset(&set));
	zassert_ok(sigfillset(&target));

	signo = SIGHUP;
	zassert_ok(sigdelset(&set, signo));
	WRITE_BIT(_target[0], signo - 1, 0);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	signo = SIGSYS;
	zassert_ok(sigdelset(&set, signo));
	WRITE_BIT(_target[0], signo - 1, 0);
	for (int i = 0; i < SIGSET_NLONGS; i++) {
		zassert_equal(_set[i], _target[i],
			      "set.sig[%d of %d] has content: %lx, expected %lx", i,
			      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
	}

	/* TODO: move rt signal tests to realtime_signals testsuite */
	const int rtsigs[] = {SIGRTMIN, SIGRTMAX};

	ARRAY_FOR_EACH(rtsigs, i) {
		int expected_ret = 0;
		int expected_errno = 0;

		signo = rtsigs[i];
		if ((signo - 1) >= SIGSET_NLONGS * BITS_PER_LONG) {
			/* Some libc's provide a sigset_t that is too small for real-time signals */
			expected_ret = -1;
			expected_errno = EINVAL;
		} else {
			WRITE_BIT(_target[(signo - 1) / BITS_PER_LONG], (signo - 1) % BITS_PER_LONG,
				  0);
		}

		errno = 0;
		zassert_equal(sigdelset(&set, signo), expected_ret);
		zassert_equal(errno, expected_errno);
		for (int i = 0; i < SIGSET_NLONGS; i++) {
			zassert_equal(_set[i], _target[i],
				      "set.sig[%d of %d] has content: %lx, expected %lx", i,
				      (int)(SIGSET_NLONGS - 1), _set[i], _target[i]);
		}
	}
}

ZTEST(posix_signals, test_sigismember_oor)
{
	int res;
	sigset_t set = {0};

	res = sigismember(&set, -1);
	zassert_equal(res, -1, "rc should be -1 but is %d", res);
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");

	res = sigismember(&set, 0);
	zassert_equal(res, -1, "rc should be -1 but is %d", res);
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");

	res = sigismember(&set, SIGRTMAX + 1);
	zassert_equal(res, -1, "rc should be -1 but is %d", res);
	zassert_equal(errno, EINVAL, "errno should be %s", "EINVAL");
}

ZTEST(posix_signals, test_sigismember)
{
	sigset_t set = (sigset_t){0};
	unsigned long *const _set = (unsigned long *)&set;

	_set[0] = BIT(SIGHUP - 1) | BIT(SIGSYS - 1);

	zassert_equal(sigismember(&set, SIGHUP), 1, "%s expected to be member", "SIGHUP");
	zassert_equal(sigismember(&set, SIGSYS), 1, "%s expected to be member", "SIGSYS");

	zassert_equal(sigismember(&set, SIGKILL), 0, "%s not expected to be member", "SIGKILL");
	zassert_equal(sigismember(&set, SIGTERM), 0, "%s not expected to be member", "SIGTERM");

	/* TODO: move rt signal tests to realtime_signals testsuite */
	const int rtsigs[] = {SIGRTMIN, SIGRTMAX};

	ARRAY_FOR_EACH(rtsigs, i) {
		int expected_ret = 1;
		int expected_errno = 0;
		int signo = rtsigs[i];

		if ((signo - 1) >= SIGSET_NLONGS * BITS_PER_LONG) {
			/* Some libc's provide a sigset_t that is too small for real-time signals */
			expected_ret = -1;
			expected_errno = EINVAL;
		} else {
			WRITE_BIT(_set[(signo - 1) / BITS_PER_LONG], (signo - 1) % BITS_PER_LONG,
				  1);
		}

		errno = 0;
		zassert_equal(sigismember(&set, signo), expected_ret);
		zassert_equal(errno, expected_errno);
	}
}

ZTEST(posix_signals, test_pthread_sigmask)
{
	test_pthread_sigmask_common(pthread_sigmask);
}

ZTEST(posix_signals, test_sigprocmask)
{
	if (IS_ENABLED(CONFIG_MULTITHREADING)) {
		if (!IS_ENABLED(CONFIG_ASSERT)) {
			zassert_not_ok(sigprocmask(SIG_SETMASK, NULL, NULL));
			zassert_equal(errno, ENOSYS);
		}
	} else {
		test_pthread_sigmask_common(sigprocmask);
	}
}

ZTEST(posix_signals, test_reserved_rt_signals)
{
	sigset_t set;

	sigemptyset(&set);
	zassert_equal(sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER), -1);
	zassert_equal(errno, EINVAL);
	zassert_equal(sigaddset(&set, CONFIG_THREAD_CANCEL_SIGNAL_NUMBER + 1), -1);
	zassert_equal(errno, EINVAL);
	if ((SIGRTMIN - 1) < SIGSET_NLONGS * BITS_PER_LONG) {
		zassert_ok(sigaddset(&set, SIGRTMIN));
	}
}

static void *setup(void)
{
	test_pthread_sigmask_init_expectation_mask();
	return NULL;
}

ZTEST_SUITE(posix_signals, NULL, setup, NULL, NULL, NULL);

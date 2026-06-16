/*
 * Copyright (c) 2023 Meta
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

typedef int (*sigmask_fn)(int how, const sigset_t *set, sigset_t *oset);

/* Adjust sigfillset() mask to match what pthread_sigmask round-trips through the kernel. */
static ZTEST_BMEM sigset_t sigfillset_mask_bits;
static inline void adjust_mask_expectation(sigset_t *set)
{
	int i;
	unsigned long *s;
	const unsigned long *m;

	for (i = 0, s = (unsigned long *)set, m = (const unsigned long *)&sigfillset_mask_bits;
	     i < SIGSET_NLONGS; i++, s++, m++) {
		*s &= *m;
	}
}

static void pr_sigset(const char *const label, const sigset_t *set)
{
	const unsigned long *const ulset = (unsigned long *)set;

	printf("%s: ", label);
	for (size_t i = SIGSET_NLONGS; i > 0; --i) {
		printf("%lx", ulset[i - 1]);
	}
	printf("\n");
}

static inline void prfmt_sigset(char *buf, size_t len, const sigset_t *set)
{
	const unsigned long *const ulset = (const unsigned long *)set;
	const size_t delta = IS_ENABLED(CONFIG_64BIT) ? 16 : 8;
	const char *fmt = IS_ENABLED(CONFIG_64BIT) ? "%016lx" : "%08lx";

	for (size_t i = SIGSET_NLONGS; i > 0; --i, buf += delta, len -= delta) {
		snprintf(buf, delta + 1, fmt, ulset[i - 1]);
	}
}

#define zassert_sigset_equal(a, b, ...)                                                            \
	do {                                                                                       \
		char buf_a[2 * SIGSET_SIZE + 1];                                                   \
		char buf_b[2 * SIGSET_SIZE + 1];                                                   \
		adjust_mask_expectation((a));                                                      \
		adjust_mask_expectation((b));                                                      \
		prfmt_sigset(buf_a, sizeof(buf_a), (a));                                           \
		prfmt_sigset(buf_b, sizeof(buf_b), (b));                                           \
		zassert_mem_equal((a), (b), SIGSET_SIZE, #a "(%s) != " #b " (%s)", buf_a, buf_b);  \
	} while (0)

#define zexpect_sigset_equal(a, b, ...)                                                            \
	do {                                                                                       \
		char buf_a[2 * SIGSET_SIZE + 1];                                                   \
		char buf_b[2 * SIGSET_SIZE + 1];                                                   \
		adjust_mask_expectation((a));                                                      \
		adjust_mask_expectation((b));                                                      \
		prfmt_sigset(buf_a, sizeof(buf_a), (a));                                           \
		prfmt_sigset(buf_b, sizeof(buf_b), (b));                                           \
		zexpect_mem_equal((a), (b), SIGSET_SIZE, #a "(%s) != " #b " (%s)", buf_a, buf_b);  \
	} while (0)

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

static void test_sigmask_common(void *arg)
{
/* for clarity */
#define SIG_GETMASK SIG_SETMASK

	enum {
		NEW,
		OLD,
	};
	sigset_t set[2];
	const int invalid_how = 0x9a2ba9e;
	sigmask_fn sigmask = arg;

	/* invalid how results in EINVAL */
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* glibc / Linux do not return EINVAL when the "how" argument is invalid (POSIX
		 * non-conformance)
		 */
		zassert_equal(sigmask(invalid_how, NULL, NULL), EINVAL);
		zassert_equal(sigmask(invalid_how, &set[NEW], &set[OLD]), EINVAL);
	}

	/* verify setting / getting masks */
	zassert_ok(sigemptyset(&set[NEW]));
	zassert_ok(sigmask(SIG_SETMASK, &set[NEW], NULL));
	zassert_ok(sigfillset(&set[OLD]));
	zassert_ok(sigmask(SIG_GETMASK, NULL, &set[OLD]));
	zassert_sigset_equal(&set[OLD], &set[NEW]);

	zassert_ok(sigfillset(&set[NEW]));
	zassert_ok(sigmask(SIG_SETMASK, &set[NEW], NULL));
	zassert_ok(sigemptyset(&set[OLD]));
	zassert_ok(sigmask(SIG_GETMASK, NULL, &set[OLD]));
	zassert_sigset_equal(&set[OLD], &set[NEW]);

	/* start with an empty mask */
	zassert_ok(sigemptyset(&set[NEW]));
	zassert_ok(sigmask(SIG_SETMASK, &set[NEW], NULL));

	/* verify SIG_BLOCK: expect (SIGUSR1 | SIGUSR2 | SIGHUP) */
	zassert_ok(sigemptyset(&set[NEW]));
	zassert_ok(sigaddset(&set[NEW], SIGUSR1));
	zassert_ok(sigmask(SIG_BLOCK, &set[NEW], NULL));

	zassert_ok(sigemptyset(&set[NEW]));
	zassert_ok(sigaddset(&set[NEW], SIGUSR2));
	zassert_ok(sigaddset(&set[NEW], SIGHUP));
	zassert_ok(sigmask(SIG_BLOCK, &set[NEW], NULL));

	zassert_ok(sigemptyset(&set[OLD]));
	zassert_ok(sigaddset(&set[OLD], SIGUSR1));
	zassert_ok(sigaddset(&set[OLD], SIGUSR2));
	zassert_ok(sigaddset(&set[OLD], SIGHUP));

	zassert_ok(sigmask(SIG_GETMASK, NULL, &set[NEW]));
	zassert_sigset_equal(&set[NEW], &set[OLD]);

	/* start with full mask */
	zassert_ok(sigfillset(&set[NEW]));
	zassert_ok(sigmask(SIG_SETMASK, &set[NEW], NULL));

	/* verify SIG_UNBLOCK: expect ~(SIGUSR1 | SIGUSR2 | SIGHUP) */
	zassert_ok(sigemptyset(&set[NEW]));
	zassert_ok(sigaddset(&set[NEW], SIGUSR1));
	zassert_ok(sigmask(SIG_UNBLOCK, &set[NEW], NULL));

	zassert_ok(sigemptyset(&set[NEW]));
	zassert_ok(sigaddset(&set[NEW], SIGUSR2));
	zassert_ok(sigaddset(&set[NEW], SIGHUP));
	zassert_ok(sigmask(SIG_UNBLOCK, &set[NEW], NULL));

	zassert_ok(sigfillset(&set[OLD]));
	zassert_ok(sigdelset(&set[OLD], SIGUSR1));
	zassert_ok(sigdelset(&set[OLD], SIGUSR2));
	zassert_ok(sigdelset(&set[OLD], SIGHUP));
	zassert_ok(sigmask(SIG_GETMASK, NULL, &set[NEW]));
	zassert_sigset_equal(&set[NEW], &set[OLD]);
}

ZTEST(posix_signals, test_pthread_sigmask)
{
	test_sigmask_common(pthread_sigmask);
}

ZTEST(posix_signals, test_sigprocmask)
{
	if (IS_ENABLED(CONFIG_MULTITHREADING)) {
		if (!IS_ENABLED(CONFIG_ASSERT)) {
			zassert_not_ok(sigprocmask(SIG_SETMASK, NULL, NULL));
			zassert_equal(errno, ENOSYS);
		}
	} else {
		test_sigmask_common(sigprocmask);
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
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		sigfillset(&sigfillset_mask_bits);
		pthread_sigmask(SIG_SETMASK, &sigfillset_mask_bits, NULL);
		sigemptyset(&sigfillset_mask_bits);
		pthread_sigmask(SIG_SETMASK, NULL, &sigfillset_mask_bits);
		/* SIGTRAP is added _after_ setup() */
		sigdelset(&sigfillset_mask_bits, SIGTRAP);
		pr_sigset("sigfillset_mask_bits", &sigfillset_mask_bits);
		return NULL;
	}

	/* helper to convert k_sig_set to sigset_t */
	extern sigset_t *z_sig_set_to_posix_slow(const struct k_sig_set *kset, sigset_t *buf);

	struct k_sig_set buf;
	struct k_sig_set *const kmask = &buf;
	sigset_t *const pmask = &sigfillset_mask_bits;

	/* set all bits in k_sig_set */
	(void)k_sig_fillset(kmask);
	/* set as many bits as possible in current mask */
	(void)k_sig_mask(K_SIG_SETMASK, kmask, NULL);
	/* get all bits that were set in current mask */
	(void)k_sig_mask(K_SIG_SETMASK, NULL, kmask);
	/* convert k_sig_set to sigset_t */
	zassert_not_null(z_sig_set_to_posix_slow(kmask, pmask));
	pr_sigset("sigfillset_mask_bits", pmask);

	return NULL;
}

ZTEST_SUITE(posix_signals, NULL, setup, NULL, NULL, NULL);

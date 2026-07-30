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

typedef int (*test_pthread_sigmask_fn)(int how, const sigset_t *set, sigset_t *oset);

#define BYTES_PER_LONG (sizeof(unsigned long))

#if defined(CONFIG_NATIVE_LIBC)
#ifdef __GLIBC__
#define SIGSET_NLONGS (_NSIG / (BITS_PER_LONG))
#else
#error "Unknown native libc"
#endif
#else
#define SIGSET_NLONGS (sizeof(sigset_t) / sizeof(unsigned long))
#endif
#define SIGSET_SIZE (SIGSET_NLONGS * BYTES_PER_LONG)

BUILD_ASSERT(SIGSET_NLONGS > 0, "sigset_t has no storage");

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

static inline void adjust_unmaskable_signals(sigset_t *set)
{
	if (!IS_ENABLED(CONFIG_USERSPACE)) {
		return;
	}

	/*
	 * k_sig_mask() clears SIGKILL and SIGSTOP when invoked from userspace;
	 * they cannot remain blocked in the stored mask.
	 */
	(void)sigdelset(set, SIGKILL);
	(void)sigdelset(set, SIGSTOP);
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
		adjust_unmaskable_signals((a));                                                    \
		adjust_unmaskable_signals((b));                                                    \
		prfmt_sigset(buf_a, sizeof(buf_a), (a));                                           \
		prfmt_sigset(buf_b, sizeof(buf_b), (b));                                           \
		zassert_mem_equal((a), (b), SIGSET_SIZE, #a "(%s) != " #b " (%s)", buf_a, buf_b);  \
	} while (0)

void test_pthread_sigmask_init_expectation_mask(void)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		sigfillset(&sigfillset_mask_bits);
		pthread_sigmask(SIG_SETMASK, &sigfillset_mask_bits, NULL);
		sigemptyset(&sigfillset_mask_bits);
		pthread_sigmask(SIG_SETMASK, NULL, &sigfillset_mask_bits);
		/* SIGTRAP is added _after_ setup() */
		sigdelset(&sigfillset_mask_bits, SIGTRAP);
		pr_sigset("sigfillset_mask_bits", &sigfillset_mask_bits);
		return;
	}

	extern sigset_t *z_sig_set_to_posix_slow(const struct k_sig_set *kset, sigset_t *buf);

	struct k_sig_set buf;
	struct k_sig_set *const kmask = &buf;
	sigset_t *const pmask = &sigfillset_mask_bits;

	(void)k_sig_fillset(kmask);
	(void)k_sig_mask(K_SIG_SETMASK, kmask, NULL);
	(void)k_sig_mask(K_SIG_SETMASK, NULL, kmask);
	zassert_not_null(z_sig_set_to_posix_slow(kmask, pmask));
	pr_sigset("sigfillset_mask_bits", pmask);
}

void test_pthread_sigmask_common(test_pthread_sigmask_fn sigmask)
{
/* for clarity */
#define SIG_GETMASK SIG_SETMASK

	enum {
		NEW,
		OLD,
	};
	sigset_t set[2];
	const int invalid_how = 0x9a2ba9e;

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

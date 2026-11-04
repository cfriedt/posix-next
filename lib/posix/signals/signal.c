/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/util.h>

#define SIGNO_WORD_IDX(_signo) ((_signo - 1) / BITS_PER_LONG)
#define SIGNO_WORD_BIT(_signo) ((_signo - 1) & BIT_MASK(LOG2(BITS_PER_LONG)))

#define SIGSET_NLONGS (sizeof(sigset_t) / sizeof(unsigned long))

static inline bool signo_fits(int signo)
{
	/* technically, 0 is not a valid signal number, but it still fits */
	return ((signo > 0) && (signo <= SIGSET_NLONGS * BITS_PER_LONG));
}

#undef sigemptyset
int sigemptyset(sigset_t *set)
{
	*set = (sigset_t){0};
	return 0;
}

#undef sigfillset
int sigfillset(sigset_t *set)
{
	unsigned long *const _set = (unsigned long *)set;

	for (int i = 0; i < SIGSET_NLONGS; i++) {
		_set[i] = -1;
	}

	for (int signo = CONFIG_THREAD_CANCEL_SIGNAL_NUMBER; signo < SIGRTMIN;
	     signo++) {
		if ((signo > 0) && ((signo - 1) < SIGSET_NLONGS * BITS_PER_LONG)) {
			WRITE_BIT(_set[SIGNO_WORD_IDX(signo)], SIGNO_WORD_BIT(signo), 0);
		}
	}

	return 0;
}

#undef sigaddset
int sigaddset(sigset_t *set, int signo)
{
	unsigned long *_set = (unsigned long *)set;

	if (!z_sig_posix_valid(signo)) {
		errno = EINVAL;
		return -1;
	}

	if (!signo_fits(signo)) {
		errno = EINVAL;
		return -1;
	}

	WRITE_BIT(_set[SIGNO_WORD_IDX(signo)], SIGNO_WORD_BIT(signo), 1);

	return 0;
}

#undef sigdelset
int sigdelset(sigset_t *set, int signo)
{
	unsigned long *_set = (unsigned long *)set;

	if (!z_sig_posix_valid(signo)) {
		errno = EINVAL;
		return -1;
	}

	if (!signo_fits(signo)) {
		errno = EINVAL;
		return -1;
	}

	WRITE_BIT(_set[SIGNO_WORD_IDX(signo)], SIGNO_WORD_BIT(signo), 0);

	return 0;
}

#undef sigismember
int sigismember(const sigset_t *set, int signo)
{
	const unsigned long *const _set = (const unsigned long *)set;

	if (!z_sig_posix_valid(signo)) {
		errno = EINVAL;
		return -1;
	}

	if (!signo_fits(signo)) {
		errno = EINVAL;
		return -1;
	}

	return 1 & (_set[SIGNO_WORD_IDX(signo)] >> SIGNO_WORD_BIT(signo));
}

static void ksigset_to_posix(sigset_t *dst, const struct k_sig_set *ksrc)
{
	sigset_t buf;
	const sigset_t *const src = z_sig_set_to_posix(ksrc, &buf);

	if (src != dst) {
		*dst = *src;
	}
}

int sigaction(int sig, const struct sigaction *ZRESTRICT act, struct sigaction *ZRESTRICT oact)
{
	int ret;
	int ksigno;
	struct k_sig_action kact;
	struct k_sig_action koact;

	if (!z_sig_posix_valid(sig)) {
		errno = EINVAL;
		return -1;
	}

	ksigno = z_sig_from_posix(sig);
	if (ksigno <= 0) {
		errno = EINVAL;
		return -1;
	}

	if (act != NULL) {
		z_sig_action_from_posix(act, &kact);
	}

	ret = k_sig_action(ksigno, (act == NULL) ? NULL : &kact, &koact);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	/* koact still describes the disposition in force before this call */
	if (oact != NULL) {
		z_sig_action_to_posix(&koact, oact);
	}

	return 0;
}

int sigprocmask(int how, const sigset_t *ZRESTRICT set, sigset_t *ZRESTRICT oset)
{
	/* make equivalent until Zephyr has processes */
	int ret = z_sig_posix_mask(how, set, oset);

	if (ret != 0) {
		errno = ret;
		return -1;
	}

	return 0;
}

int sigpending(sigset_t *set)
{
	int ret;
	struct k_sig_set kset;

	if (set == NULL) {
		errno = EINVAL;
		return -1;
	}

	ret = k_sig_pending(&kset);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	ksigset_to_posix(set, &kset);

	return 0;
}

int pause(void)
{
	z_sig_wait_delivered(k_sig_delivered());

	errno = EINTR;

	return -1;
}

int sigsuspend(const sigset_t *sigmask)
{
	int ret;
	unsigned long start;
	struct k_sig_set kmask_buf;
	struct k_sig_set kold;
	const struct k_sig_set *kmask;

	if (sigmask == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* sample early to catch a pending signal */
	start = k_sig_delivered();

	kmask = z_sig_set_from_posix(sigmask, &kmask_buf);

	ret = k_sig_mask(K_SIG_SETMASK, kmask, &kold);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	z_sig_wait_delivered(start);

	/* the mask in force before the call is always restored */
	(void)k_sig_mask(K_SIG_SETMASK, &kold, NULL);

	errno = EINTR;

	return -1;
}

int sigwait(const sigset_t *ZRESTRICT set, int *ZRESTRICT sig)
{
	int ret;
	struct k_sig_info kinfo;
	struct k_sig_set kset_buf;
	const struct k_sig_set *kset;

	if ((set == NULL) || (sig == NULL)) {
		return EINVAL;
	}

	kset = z_sig_set_from_posix(set, &kset_buf);

	do {
		ret = k_sig_timedwait(kset, &kinfo, K_MSEC(100));
	} while (ret == -EAGAIN);

	if (ret < 0) {
		return -ret;
	}

	*sig = z_sig_ksigno_to_posix(kinfo.signo);

	return 0;
}

int kill(pid_t pid, int sig)
{
	int ret;
	int ksigno;

	if (sig == 0) {
		ksigno = 0;
	} else {
		if (!z_sig_posix_valid(sig)) {
			errno = EINVAL;
			return -1;
		}

		ksigno = z_sig_from_posix(sig);
		if (ksigno <= 0) {
			errno = EINVAL;
			return -1;
		}
	}

	ret = z_sig_posix_kill(pid, ksigno);

	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}
#ifdef CONFIG_POSIX_SIGNALS_ALIAS_KILL
FUNC_ALIAS(kill, _kill, int);
#endif /* CONFIG_POSIX_SIGNALS_ALIAS_KILL */

unsigned int alarm(unsigned int seconds)
{
	k_ticks_t remaining =
		k_sig_alarm((seconds == 0) ? K_FOREVER : K_SECONDS(seconds));

	if (remaining <= 0) {
		return 0;
	}

	/* the kernel arms a relative timeout one tick past its deadline: not time left */
	return MAX(1U, (unsigned int)DIV_ROUND_UP(k_ticks_to_ms_floor64((uint64_t)(remaining - 1)),
						  MSEC_PER_SEC));
}

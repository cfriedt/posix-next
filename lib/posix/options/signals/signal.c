/*
 * Copyright (c) 2023 Meta
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "posix_internal.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/util.h>

#define SIGNO_WORD_IDX(_signo) ((_signo - 1) / BITS_PER_LONG)
#define SIGNO_WORD_BIT(_signo) ((_signo - 1) & BIT_MASK(LOG2(BITS_PER_LONG)))

#define SIGSET_NLONGS (sizeof(sigset_t) / sizeof(unsigned long))

static inline bool posix_sig_is_reserved(int signo)
{
	return signo == CONFIG_THREAD_CANCEL_SIGNAL_NUMBER ||
	       signo == (CONFIG_THREAD_CANCEL_SIGNAL_NUMBER + 1);
}

static inline bool signo_valid(int signo)
{
	if (signo <= 0 || posix_sig_is_reserved(signo)) {
		return false;
	}

	if (signo < SIGRTMIN) {
		return signo <= 31;
	}

	return signo <= SIGRTMAX;
}

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

	if (!signo_valid(signo)) {
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

	if (!signo_valid(signo)) {
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

	if (!signo_valid(signo)) {
		errno = EINVAL;
		return -1;
	}

	if (!signo_fits(signo)) {
		errno = EINVAL;
		return -1;
	}

	return 1 & (_set[SIGNO_WORD_IDX(signo)] >> SIGNO_WORD_BIT(signo));
}

static int ksigno_to_posix(int ksigno)
{
	int posix = z_sig_to_posix(ksigno);

	return (posix > 0) ? posix : ksigno;
}

static void ksigset_to_posix(sigset_t *dst, const struct k_sig_set *ksrc)
{
	sigset_t buf;
	const sigset_t *const src = z_sig_set_to_posix(ksrc, &buf);

	if (src != dst) {
		*dst = *src;
	}
}

static void kinfo_to_siginfo(siginfo_t *dst, const struct k_sig_info *src, int signo)
{
	*dst = (siginfo_t){0};
	dst->si_signo = signo;
	dst->si_code = src->code;
	dst->si_value.sival_int = src->value.sival_int;
}

/* handle a POSIX sigaction from a kernel signal handler */
void z_sig_shim(int ksigno, struct k_sig_info *kinfo, void *context)
{
	void *unused;
	struct k_sig_info kinfo_unused;
	struct k_sig_action action;
	const int signo = ksigno_to_posix(ksigno);

	ARG_UNUSED(kinfo);

	k_sig_current(&kinfo_unused, &action, &unused);

	if (action.user_data == NULL) {
		return;
	}

	if ((action.flags & K_SIG_SA_SIGINFO) != 0) {
		siginfo_t info;

		kinfo_to_siginfo(&info, kinfo, signo);
		((void (*)(int, siginfo_t *, void *))action.user_data)(signo, &info, context);
	} else {
		((void (*)(int))action.user_data)(signo);
	}
}

/* reconstruct the POSIX action previously in force from the kernel's record of it */
static void posix_sig_disposition_get(const struct k_sig_action *koact, struct sigaction *oact)
{
	*oact = (struct sigaction){0};
	ksigset_to_posix(&oact->sa_mask, &koact->mask);

	if (koact->handler == z_sig_shim) {
		oact->sa_handler = (void (*)(int))koact->user_data;
	} else if ((koact->handler != K_SIG_DFL) && (koact->handler != K_SIG_IGN)) {
		/* registered with the kernel unwrapped, e.g. through the k_sig_* API directly */
		oact->sa_handler = (void (*)(int))koact->handler;
	} else {
		oact->sa_handler = (koact->handler == K_SIG_IGN) ? SIG_IGN : SIG_DFL;
	}

	if ((koact->flags & K_SIG_SA_SIGINFO) != 0) {
		oact->sa_flags |= SA_SIGINFO;
	}
	if ((koact->flags & K_SIG_SA_RESETHAND) != 0) {
		oact->sa_flags |= SA_RESETHAND;
	}
	if ((koact->flags & K_SIG_SA_NODEFER) != 0) {
		oact->sa_flags |= SA_NODEFER;
	}
}

int sigaction(int sig, const struct sigaction *ZRESTRICT act, struct sigaction *ZRESTRICT oact)
{
	int ret;
	int ksigno;
	struct k_sig_action kact;
	struct k_sig_action koact;
	struct k_sig_set kmask_buf;

	if (!signo_valid(sig)) {
		errno = EINVAL;
		return -1;
	}

	ksigno = z_sig_from_posix(sig);
	if (ksigno <= 0) {
		errno = EINVAL;
		return -1;
	}

	if (act != NULL) {
		kact = (struct k_sig_action){0};
		if ((act->sa_handler == SIG_DFL) || (act->sa_handler == SIG_IGN)) {
			kact.handler = (act->sa_handler == SIG_IGN) ? K_SIG_IGN : K_SIG_DFL;
		} else {
			kact.handler = z_sig_shim;
			kact.user_data = (void *)act->sa_handler;
		}

		if ((act->sa_flags & SA_SIGINFO) != 0) {
			kact.flags |= K_SIG_SA_SIGINFO;
		}
		if ((act->sa_flags & SA_RESETHAND) != 0) {
			kact.flags |= K_SIG_SA_RESETHAND;
		}
		if ((act->sa_flags & SA_NODEFER) != 0) {
			kact.flags |= K_SIG_SA_NODEFER;
		}

		kact.mask = *z_sig_set_from_posix(&act->sa_mask, &kmask_buf);
	}

	ret = k_sig_action(ksigno, (act == NULL) ? NULL : &kact, &koact);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	/* koact still describes the disposition in force before this call */
	if (oact != NULL) {
		posix_sig_disposition_get(&koact, oact);
	}

	return 0;
}

int sigprocmask(int how, const sigset_t *ZRESTRICT set, sigset_t *ZRESTRICT oset)
{
	/* make equivalent until Zephyr has processes */
	int ret = pthread_sigmask(how, set, oset);

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

/*
 * Block until a signal has been delivered to a handler in the calling thread, detected by
 * sampling the thread's kernel delivery count against 'start'.
 */
static void posix_sig_wait_delivered(unsigned long start)
{
	while (k_sig_delivered() == start) {
		k_sleep(K_TICKS(1));
	}
}

int pause(void)
{
	posix_sig_wait_delivered(k_sig_delivered());

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

	posix_sig_wait_delivered(start);

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

	*sig = ksigno_to_posix(kinfo.signo);

	return 0;
}

int kill(pid_t pid, int sig)
{
	int ret;
	int ksigno;
	k_tid_t tid;
	pthread_t th = (pthread_t)(uintptr_t)pid;

	if (sig == 0) {
		ksigno = 0;
	} else {
		if (!signo_valid(sig)) {
			errno = EINVAL;
			return -1;
		}

		ksigno = z_sig_from_posix(sig);
		if (ksigno <= 0) {
			errno = EINVAL;
			return -1;
		}
	}

	if (pid == getpid()) {
		tid = k_current_get();
	} else if (pid > 0) {
		tid = to_k_thread(&th);
	} else {
		/* Zephyr does not yet support process groups (pid <= 0) */
		errno = ESRCH;
		return -1;
	}

	ret = k_sig_queue(tid, ksigno, (union k_sig_val){0});
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

	return (unsigned int)DIV_ROUND_UP(k_ticks_to_ms_ceil64((uint64_t)remaining),
					  MSEC_PER_SEC);
}

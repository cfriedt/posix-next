/*
 * Copyright (c) 2024, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>

static int ksigno_to_posix(int ksigno)
{
	int posix = z_sig_to_posix(ksigno);

	return (posix > 0) ? posix : ksigno;
}

static void kinfo_to_siginfo(siginfo_t *dst, const struct k_sig_info *src)
{
	if (dst == NULL) {
		return;
	}

	*dst = (siginfo_t){0};
	dst->si_signo = ksigno_to_posix(src->signo);
	dst->si_code = src->code;
	dst->si_value.sival_int = src->value.sival_int;
}

int sigqueue(pid_t pid, int signo, union sigval value)
{
	pthread_t th = (pthread_t)(uintptr_t)pid;
	struct k_thread *tid = to_k_thread(&th);
	union k_sig_val val = {
		.sival_int = value.sival_int,
	};
	int ret;

	if ((signo < 0) || (signo > SIGNAL_SET_SIZE)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Zephyr maps pid to pthread_t (a k_thread pointer). On 32-bit targets
	 * that handle is often >= 0x80000000, so casting to signed pid_t is
	 * negative even though the thread is valid. Reject only explicit invalid
	 * targets; k_sig_queue() validates the resolved thread.
	 */
	if ((pid == 0) || (pid == (pid_t)-1)) {
		errno = ESRCH;
		return -1;
	}

	signo = z_sig_from_posix(signo);
	if (signo < 0) {
		errno = EINVAL;
		return -1;
	}

	ret = k_sig_queue((k_pid_t)tid, signo, val);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

static int sigwait_poll(const struct k_sig_set *kset, struct k_sig_info *kinfo, k_timeout_t timeout)
{
	int ret;

	/*
	 * k_sig_timedwait(..., K_FOREVER) can return immediately when no signal
	 * is queued yet; poll in finite chunks (see kernel signal tests).
	 */
	if (K_TIMEOUT_EQ(timeout, K_FOREVER)) {
		do {
			ret = k_sig_timedwait(kset, kinfo, K_MSEC(100));
		} while (ret == -EAGAIN);

		return ret;
	}

	return k_sig_timedwait(kset, kinfo, timeout);
}

int sigtimedwait(const sigset_t *ZRESTRICT set, siginfo_t *ZRESTRICT info,
		 const struct timespec *ZRESTRICT timeout)
{
	struct k_sig_info kinfo;
	struct k_sig_set kset_buf;
	const struct k_sig_set *kset;
	k_timeout_t to;
	int ret;

	if (set == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (timeout == NULL) {
		to = K_FOREVER;
	} else {
		to = K_MSEC(timeout->tv_sec * MSEC_PER_SEC + timeout->tv_nsec / NSEC_PER_MSEC);
	}

	kset = z_sig_set_from_posix(set, &kset_buf);
	ret = sigwait_poll(kset, info != NULL ? &kinfo : NULL, to);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	if (info != NULL) {
		kinfo_to_siginfo(info, &kinfo);
	}

	return ksigno_to_posix(ret);
}

int sigwaitinfo(const sigset_t *ZRESTRICT set, siginfo_t *ZRESTRICT info)
{
	struct k_sig_info kinfo;
	struct k_sig_set kset_buf;
	const struct k_sig_set *kset;
	int ret;

	if (set == NULL) {
		errno = EINVAL;
		return -1;
	}

	kset = z_sig_set_from_posix(set, &kset_buf);
	ret = sigwait_poll(kset, info != NULL ? &kinfo : NULL, K_FOREVER);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	if (info != NULL) {
		kinfo_to_siginfo(info, &kinfo);
	}

	return ksigno_to_posix(ret);
}

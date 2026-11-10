/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_internal.h"

#include <errno.h>
#include <signal.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>

/* the kernel number of POSIX signal sig, or -1 with errno set */
static int ksigno_of(int sig)
{
	int ksigno = z_sig_posix_valid(sig) ? z_sig_from_posix(sig) : -1;

	if (ksigno <= 0) {
		errno = EINVAL;
		return -1;
	}

	return ksigno;
}

static int result(int ret)
{
	if (ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

int killpg(pid_t pgrp, int sig)
{
	int ksigno = 0;

	if (pgrp < 0) {
		errno = EINVAL;
		return -1;
	}
	if (sig != 0) {
		ksigno = ksigno_of(sig);
		if (ksigno < 0) {
			return -1;
		}
	}

	return result(z_sig_posix_kill(-pgrp, ksigno));
}

int sigaltstack(const stack_t *ZRESTRICT ss, stack_t *ZRESTRICT oss)
{
	struct k_sig_stack kss;
	struct k_sig_stack koss;
	int ret;

	if (ss != NULL) {
		if ((ss->ss_flags & ~SS_DISABLE) != 0) {
			errno = EINVAL;
			return -1;
		}
		kss = (struct k_sig_stack){
			.sp = ss->ss_sp,
			.size = ss->ss_size,
			.flags = ((ss->ss_flags & SS_DISABLE) != 0) ? K_SIG_SS_DISABLE : 0,
		};
		if ((kss.flags == 0) && (kss.size < MINSIGSTKSZ)) {
			errno = ENOMEM;
			return -1;
		}
	}

	ret = k_sig_altstack((ss != NULL) ? &kss : NULL, (oss != NULL) ? &koss : NULL);
	if (ret < 0) {
		return result(ret);
	}

	if (oss != NULL) {
		*oss = (stack_t){
			.ss_sp = koss.sp,
			.ss_size = koss.size,
			.ss_flags = (((koss.flags & K_SIG_SS_ONSTACK) != 0) ? SS_ONSTACK : 0) |
				    (((koss.flags & K_SIG_SS_DISABLE) != 0) ? SS_DISABLE : 0),
		};
	}

	return 0;
}

static int mask_one(int how, int sig)
{
	struct k_sig_set set;
	int ksigno = ksigno_of(sig);

	if (ksigno < 0) {
		return -1;
	}
	k_sig_emptyset(&set);
	(void)k_sig_addset(&set, ksigno);

	return result(k_sig_mask(how, &set, NULL));
}

int sighold(int sig)
{
	return mask_one(K_SIG_BLOCK, sig);
}

int sigrelse(int sig)
{
	return mask_one(K_SIG_UNBLOCK, sig);
}

int sigignore(int sig)
{
	int ksigno = ksigno_of(sig);

	if (ksigno < 0) {
		return -1;
	}

	return result(k_sig_action(ksigno, &(struct k_sig_action){.handler = K_SIG_IGN}, NULL));
}

int siginterrupt(int sig, int flag)
{
	struct k_sig_action kact;
	int ksigno = ksigno_of(sig);
	int ret;

	ARG_UNUSED(flag);

	if (ksigno < 0) {
		return -1;
	}

	/* the kernel restarts no interrupted call: only the disposition must be one that may be set */
	ret = k_sig_action(ksigno, NULL, &kact);
	if (ret == 0) {
		ret = k_sig_action(ksigno, &kact, NULL);
	}

	return result(ret);
}

int sigpause(int sig)
{
	struct k_sig_set mask;
	struct k_sig_set old;
	unsigned long start;
	int ksigno = ksigno_of(sig);
	int ret;

	if (ksigno < 0) {
		return -1;
	}

	/* sample early to catch a signal already pending */
	start = k_sig_delivered();
	ret = k_sig_mask(K_SIG_BLOCK, NULL, &mask);
	if (ret == 0) {
		(void)k_sig_delset(&mask, ksigno);
		ret = k_sig_mask(K_SIG_SETMASK, &mask, &old);
	}
	if (ret < 0) {
		return result(ret);
	}

	z_sig_wait_delivered(start);

	/* the mask in force before the call is always restored */
	(void)k_sig_mask(K_SIG_SETMASK, &old, NULL);

	errno = EINTR;

	return -1;
}

void (*sigset(int sig, void (*disp)(int)))(int)
{
	struct k_sig_set mask;
	struct k_sig_action kact;
	struct sigaction oact;
	int ksigno = ksigno_of(sig);
	int held;
	int ret;

	if (ksigno < 0) {
		return SIG_ERR;
	}

	ret = k_sig_mask(K_SIG_BLOCK, NULL, &mask);
	if (ret == 0) {
		ret = k_sig_action(ksigno, NULL, &kact);
	}
	if (ret < 0) {
		(void)result(ret);
		return SIG_ERR;
	}
	held = k_sig_ismember(&mask, ksigno);
	z_sig_action_to_posix(&kact, &oact);

	if (disp == SIG_HOLD) {
		ret = mask_one(K_SIG_BLOCK, sig);
	} else {
		struct sigaction act = {.sa_handler = disp};

		z_sig_action_from_posix(&act, &kact);
		ret = result(k_sig_action(ksigno, &kact, NULL));
		if (ret == 0) {
			ret = mask_one(K_SIG_UNBLOCK, sig);
		}
	}
	if (ret < 0) {
		return SIG_ERR;
	}

	return (held == 1) ? SIG_HOLD : oact.sa_handler;
}

/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* the parts of POSIX signal handling more than one signal option group needs */

#include "posix_internal.h"

#include <errno.h>
#include <signal.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/signal.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/util.h>

#ifdef CONFIG_PROCESS
/* POSIX kill() pid encodings decode here; the kernel is handle-domain only */
static int kill_multi(pid_t pid, int ksigno)
{
	if (pid > 0) {
		k_pid_t proc = sys_process_find((int)pid);

		return (proc == NULL) ? -ESRCH : k_kill(proc, ksigno);
	}
	if (pid == 0) {
		/* the caller's own process group */
		return k_kill_pgrp(NULL, ksigno);
	}
	if (pid == -1) {
		/* every process the caller may signal, except itself */
		return k_kill_all(ksigno);
	}

	k_pgrp_t grp = sys_pgrp_find((int)-pid);

	return (grp == NULL) ? -ESRCH : k_kill_pgrp(grp, ksigno);
}
#else
static inline int kill_multi(pid_t pid, int ksigno)
{
	ARG_UNUSED(pid);
	ARG_UNUSED(ksigno);
	return -ESRCH;
}
#endif /* CONFIG_PROCESS */

int z_sig_posix_kill(pid_t pid, int ksigno)
{
	int ret;

	if (IS_ENABLED(CONFIG_PROCESS)) {
		/* multi-process: decode the pid encodings onto the handle-domain kernel */
		ret = kill_multi(pid, ksigno);
	} else if ((pid == POSIX_THIS_PID) || (pid == 0) || (pid == -1)) {
		/*
		 * single-process: its pid, 0, and -1 all name the one (figurative)
		 * process, and the kernel performs process-directed delivery over
		 * it - the caller takes the signal when it has it unblocked, and
		 * it otherwise pends at system scope for the first thread that
		 * unblocks or waits for it.
		 */
		ret = k_kill(NULL, ksigno);
	} else {
		ret = -ESRCH;
	}

	return ret;
}

#if defined(CONFIG_SIGNAL_FAULT) && defined(SEGV_MAPERR)
/* a K_SEGV_* .. K_TRAP_* code of a fault-generated signal as its POSIX counterpart */
static int fault_code_to_posix(int ksigno, int code)
{
	static const int ill[] = {ILL_ILLOPC, ILL_ILLOPN, ILL_ILLADR, ILL_ILLTRP,
				  ILL_PRVOPC, ILL_PRVREG, ILL_COPROC, ILL_BADSTK};
	static const int fpe[] = {FPE_INTDIV, FPE_INTOVF, FPE_FLTDIV, FPE_FLTOVF,
				  FPE_FLTUND, FPE_FLTRES, FPE_FLTINV, FPE_FLTSUB};
	static const int segv[] = {SEGV_MAPERR, SEGV_ACCERR};
	static const int bus[] = {BUS_ADRALN, BUS_ADRERR, BUS_OBJERR};
#ifdef TRAP_BRKPT
	static const int trap[] = {TRAP_BRKPT, TRAP_TRACE};
#endif
	const int *map = NULL;
	size_t n = 0;

	switch (ksigno) {
	case K_SIG_ILL:
		map = ill;
		n = ARRAY_SIZE(ill);
		break;
	case K_SIG_FPE:
		map = fpe;
		n = ARRAY_SIZE(fpe);
		break;
	case K_SIG_SEGV:
		map = segv;
		n = ARRAY_SIZE(segv);
		break;
	case K_SIG_BUS:
		map = bus;
		n = ARRAY_SIZE(bus);
		break;
#ifdef TRAP_BRKPT
	case K_SIG_TRAP:
		map = trap;
		n = ARRAY_SIZE(trap);
		break;
#endif
	default:
		break;
	}

	return ((code > 0) && ((size_t)code <= n)) ? map[code - 1] : z_si_code_to_posix(code);
}
#endif /* CONFIG_SIGNAL_FAULT && SEGV_MAPERR */

static void kinfo_to_siginfo(siginfo_t *dst, const struct k_sig_info *src, int signo)
{
	*dst = (siginfo_t){0};
	dst->si_signo = signo;
	dst->si_code = z_si_code_to_posix(src->code);
	dst->si_value.sival_ptr = src->value.sival_ptr;
#ifdef CONFIG_SIGNAL_FAULT
	dst->si_addr = src->addr;
#ifdef SEGV_MAPERR
	dst->si_code = fault_code_to_posix(src->signo, src->code);
#endif
#endif /* CONFIG_SIGNAL_FAULT */
#ifdef CONFIG_PROCESS
	/* the child's number as captured at generation: the child may be reaped by now */
	dst->si_pid = (pid_t)src->child_id;
	if (src->code == K_SI_CLD_EXITED) {
		dst->si_status = K_WEXITSTATUS(src->status);
	} else if (src->code == K_SI_CLD_KILLED) {
		dst->si_status = z_sig_to_posix(K_WTERMSIG(src->status));
	} else if (src->code == K_SI_CLD_STOPPED) {
		dst->si_status = z_sig_to_posix(K_WSTOPSIG(src->status));
	} else if (src->code == K_SI_CLD_CONTINUED) {
		dst->si_status = SIGCONT;
	}
#endif /* CONFIG_PROCESS */
}

/* handle a POSIX sigaction from a kernel signal handler */
void z_sig_shim(int ksigno, struct k_sig_info *kinfo, void *context)
{
	void *unused;
	struct k_sig_info kinfo_unused;
	struct k_sig_action action;
	const int signo = z_sig_ksigno_to_posix(ksigno);

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

void z_sig_action_from_posix(const struct sigaction *act, struct k_sig_action *kact)
{
	struct k_sig_set kmask_buf;

	*kact = (struct k_sig_action){0};
	if ((act->sa_handler == SIG_DFL) || (act->sa_handler == SIG_IGN)) {
		kact->handler = (act->sa_handler == SIG_IGN) ? K_SIG_IGN : K_SIG_DFL;
	} else {
		kact->handler = z_sig_shim;
		kact->user_data = (void *)act->sa_handler;
	}

	if ((act->sa_flags & SA_SIGINFO) != 0) {
		kact->flags |= K_SIG_SA_SIGINFO;
	}
	if ((act->sa_flags & SA_RESETHAND) != 0) {
		kact->flags |= K_SIG_SA_RESETHAND;
	}
	if ((act->sa_flags & SA_NODEFER) != 0) {
		kact->flags |= K_SIG_SA_NODEFER;
	}
	if ((act->sa_flags & SA_NOCLDSTOP) != 0) {
		kact->flags |= K_SIG_SA_NOCLDSTOP;
	}

	kact->mask = *z_sig_set_from_posix(&act->sa_mask, &kmask_buf);
}

/* reconstruct the POSIX action previously in force from the kernel's record of it */
void z_sig_action_to_posix(const struct k_sig_action *koact, struct sigaction *oact)
{
	sigset_t buf;
	const sigset_t *const mask = z_sig_set_to_posix(&koact->mask, &buf);

	*oact = (struct sigaction){0};
	oact->sa_mask = *mask;

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
	if ((koact->flags & K_SIG_SA_NOCLDSTOP) != 0) {
		oact->sa_flags |= SA_NOCLDSTOP;
	}
}

/*
 * Block until a signal has been delivered to a handler in the calling thread, detected by
 * sampling the thread's kernel delivery count against 'start'.
 */
/* block until a signal is delivered: a wait that accepts nothing ends when one is due */
void z_sig_wait_delivered(unsigned long start)
{
	struct k_sig_set none;

	k_sig_emptyset(&none);
	while (k_sig_delivered() == start) {
		(void)k_sig_timedwait(&none, NULL, K_FOREVER);
		if (!k_is_user_context() && (k_sig_delivered() == start)) {
			/* a kernel-mode caller delivers on return from a sleep */
			k_sleep(K_TICKS(1));
		}
	}
}

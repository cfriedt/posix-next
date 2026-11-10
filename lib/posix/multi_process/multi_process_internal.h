/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_LIB_POSIX_OPTIONS_MULTI_PROCESS_INTERNAL_H_
#define ZEPHYR_LIB_POSIX_OPTIONS_MULTI_PROCESS_INTERNAL_H_

#include <stdarg.h>

#include "posix_internal.h"

#include <sys/wait.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>

/* the wait wrappers pass POSIX option flags straight to k_waitpid() */
BUILD_ASSERT(WNOHANG == K_PROCESS_WNOHANG);
BUILD_ASSERT(WUNTRACED == K_PROCESS_WUNTRACED);
BUILD_ASSERT(WEXITED == K_PROCESS_WEXITED);
BUILD_ASSERT(WNOWAIT == K_PROCESS_WNOWAIT);

/*
 * Re-encode a kernel wait status into the POSIX layout: the bit layout is
 * shared (K_WSTATUS_* / W*), but a signal number in a killed status is in the
 * kernel's numbering and must be mapped to the POSIX signal number.
 */
static inline int wstatus_to_posix(int kws)
{
	if (K_WIFSIGNALED(kws)) {
		int posix_signo = z_sig_to_posix(K_WTERMSIG(kws));

		return (kws & ~0x7f) | (posix_signo & 0x7f);
	}

	return kws;
}

/*
 * Wait, peek-first: the numbering entry for a child retires when it is reaped,
 * so its numeric pid must be read while it is still a zombie. Peek with
 * K_PROCESS_WNOWAIT, capture the number, then reap the specific child. A
 * second waiter can steal the zombie between the two steps (and the reused
 * handle may already name a running process, returning -EAGAIN from the
 * targeted no-wait reap); loop when that happens.
 */
/*
 * One wait: peek with WNOWAIT, read the child's cycles if asked (its own and its reaped
 * children's, as a zombie keeps them until the reap), then consume the report.
 */
static inline int posix_wait_common(k_pid_t child, k_pgrp_t grp, bool by_grp, pid_t *num_out,
				    int *kws_out, int options, uint64_t *cycles)
{
	int ret;
	int kws;
	k_pid_t reaped;

	while (true) {
		reaped = NULL;
		kws = 0;
		if (by_grp) {
			ret = k_waitpid_pgrp(grp, &reaped, &kws,
					     (uint32_t)options | K_PROCESS_WNOWAIT, K_FOREVER);
		} else {
			ret = k_waitpid(child, &reaped, &kws,
					(uint32_t)options | K_PROCESS_WNOWAIT, K_FOREVER);
		}
		if (ret < 0) {
			return ret;
		}

		*num_out = (pid_t)sys_process_id(reaped);
		*kws_out = kws;
		if (cycles != NULL) {
			uint64_t s = 0;
			uint64_t c = 0;

			(void)k_process_cpu_stats(reaped, &s, &c);
			*cycles = s + c;
		}

		if ((options & K_PROCESS_WNOWAIT) != 0) {
			return 0;
		}

		if (K_WIFSTOPPED(kws) || K_WIFCONTINUED(kws)) {
			/* a job report leaves nothing to reap: consume it */
			uint32_t jobopt = K_WIFSTOPPED(kws) ? K_PROCESS_WUNTRACED
							    : K_PROCESS_WCONTINUED;

			ret = k_waitpid(reaped, NULL, NULL, jobopt | K_PROCESS_WNOHANG,
					K_NO_WAIT);
			if (ret == 0) {
				return 0;
			}
			/* consumed by a concurrent waiter: go around again */
			continue;
		}

		ret = k_waitpid(reaped, NULL, NULL, 0, K_NO_WAIT);
		if (ret == 0) {
			return 0;
		}
		/* stolen by a concurrent waiter: go around again */
	}
}

/* waitpid()'s selector: a child, any child (-1), the caller's group (0), a group (< -1) */
static inline int posix_wait_dispatch(pid_t pid, pid_t *num_out, int *kws_out, int options,
				      uint64_t *cycles)
{
	if (pid == -1) {
		return posix_wait_common(NULL, NULL, false, num_out, kws_out, options, cycles);
	}
	if (pid == 0) {
		return posix_wait_common(NULL, NULL, true, num_out, kws_out, options, cycles);
	}
	if (pid < -1) {
		k_pgrp_t grp = sys_pgrp_find((int)-pid);

		if (grp == NULL) {
			return -ECHILD;
		}
		return posix_wait_common(NULL, grp, true, num_out, kws_out, options, cycles);
	}
	{
		k_pid_t child = sys_process_find((int)pid);

		if (child == NULL) {
			return -ECHILD;
		}
		return posix_wait_common(child, NULL, false, num_out, kws_out, options, cycles);
	}
}

/*
 * Collect an execl-style variadic argument list into @a argv (sized
 * CONFIG_POSIX_EXEC_ARGS_MAX + 1). On success @a ap is positioned after the
 * NULL terminator (execle's envp follows). Returns -1 when the list does not
 * fit.
 */
int z_posix_execl_argv(char **argv, const char *arg0, va_list ap);


/* in-place image replacement: prune members, reset signals, close CLOEXEC */

/* the caller's environment for a new image: the POSIX store when there is one, else the C
 * library's own, else none (the minimal C library has no environment outside the POSIX layer)
 */
static inline char **z_posix_exec_environ(void)
{
#if defined(CONFIG_POSIX_SINGLE_PROCESS) || defined(CONFIG_PICOLIBC) || defined(CONFIG_NEWLIB_LIBC)
	extern char **environ;

	return environ;
#else
	return NULL;
#endif
}

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_MULTI_PROCESS_INTERNAL_H_ */

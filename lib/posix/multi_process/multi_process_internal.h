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

#ifdef CONFIG_POSIX_EXEC_LLEXT
/* unload the extension (if any) behind a just-reaped exec'd process */
void z_posix_exec_llext_reap(k_pid_t reaped);
#else
static inline void z_posix_exec_llext_reap(k_pid_t reaped)
{
	ARG_UNUSED(reaped);
}
#endif /* CONFIG_POSIX_EXEC_LLEXT */

/*
 * Wait, peek-first: the numbering entry for a child retires when it is reaped,
 * so its numeric pid must be read while it is still a zombie. Peek with
 * K_PROCESS_WNOWAIT, capture the number, then reap the specific child. A
 * second waiter can steal the zombie between the two steps (and the reused
 * handle may already name a running process, returning -EAGAIN from the
 * targeted no-wait reap); loop when that happens.
 */
static inline int posix_wait_common(k_pid_t child, k_pgrp_t grp, bool by_grp, pid_t *num_out,
				    int *kws_out, int options)
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

		if ((options & K_PROCESS_WNOWAIT) != 0) {
			return 0;
		}

		ret = k_waitpid(reaped, NULL, NULL, 0, K_NO_WAIT);
		if (ret == 0) {
			z_posix_exec_llext_reap(reaped);
			return 0;
		}
		/* stolen by a concurrent waiter: go around again */
	}
}

/*
 * Collect an execl-style variadic argument list into @a argv (sized
 * CONFIG_POSIX_EXEC_ARGS_MAX + 1). On success @a ap is positioned after the
 * NULL terminator (execle's envp follows). Returns -1 when the list does not
 * fit.
 */
int z_posix_execl_argv(char **argv, const char *arg0, va_list ap);

const char *z_posix_exec_resolve(const char *file, char *buf, size_t buflen);

/* in-place image replacement: prune members, reset signals, close CLOEXEC */
void z_posix_exec_prepare(void);

/* a resolved image, ready to run: exactly one of ext_main/entry is set */
struct z_posix_exec_run_args {
	int (*ext_main)(int argc, char **argv, char **envp);
	k_thread_entry_t entry;
	char *const *argv;
	char *const *envp;
};

/* bound the vectors before committing to an exec: 0, or -1 with E2BIG */
int z_posix_exec_args_check(char *const argv[], char *const envp[]);

/*
 * Replace the process image with the resolved one, on a fresh pool stack
 * when one is available (the caller's own stack otherwise). Never returns.
 */
FUNC_NORETURN void z_posix_exec_run(const struct z_posix_exec_run_args *args);

#ifdef CONFIG_POSIX_EXEC_LLEXT
/* load @a path as an ELF extension and run its main() as the new image */
int z_posix_exec_llext(const char *path, char *const argv[], char *const envp[]);
#endif /* CONFIG_POSIX_EXEC_LLEXT */

#endif /* ZEPHYR_LIB_POSIX_OPTIONS_MULTI_PROCESS_INTERNAL_H_ */

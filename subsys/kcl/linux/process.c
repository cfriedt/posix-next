/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* process identity, exit, limits and the system name */

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/kernel/process.h>
#include <zephyr/sys/process.h>
#include <zephyr/sys/zvfs.h>
#include <zephyr/kernel_version.h>
#include <zephyr/version.h>

#include "kcl_linux.h"

#define RLIMIT_STACK 3
#define RLIMIT_NOFILE 7
#define RLIM_NLIMITS 16
#define RLIM_INFINITY (~0ULL)

/* the process layer closes the descriptors at termination, as Linux does */
static void exit_process(int error_code)
{
	k_exit(error_code);
	CODE_UNREACHABLE;
}

ZKCL_LINUX_IMPL(exit_group)(int error_code)
{
	exit_process(error_code);
	CODE_UNREACHABLE;
}

ZKCL_LINUX_IMPL(exit)(int error_code)
{
	/* the leader is the only thread until clone() makes threads */
	exit_process(error_code);
	CODE_UNREACHABLE;
}

ZKCL_LINUX_IMPL(getpid)(void)
{
	return sys_process_id(k_getpid());
}

ZKCL_LINUX_IMPL(getppid)(void)
{
	return sys_process_id(k_getppid());
}

ZKCL_LINUX_IMPL(gettid)(void)
{
	return sys_thread_id(_current);
}

ZKCL_LINUX_IMPL(getuid)(void)
{
	return 0;
}

ZKCL_LINUX_IMPL(geteuid)(void)
{
	return 0;
}

ZKCL_LINUX_IMPL(getgid)(void)
{
	return 0;
}

ZKCL_LINUX_IMPL(getegid)(void)
{
	return 0;
}

ZKCL_LINUX_IMPL(getpgid)(zkcl_linux_pid_t pid)
{
	k_pid_t proc = (pid == 0) ? k_getpid() : sys_process_find(pid);

	if (proc == NULL) {
		return -ESRCH;
	}

	return sys_pgrp_id(k_getpgid(proc));
}

ZKCL_LINUX_IMPL(set_tid_address)(int *tidptr)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	if (p != NULL) {
		p->clear_child_tid = (uintptr_t)tidptr;
	}

	return sys_thread_id(_current);
}

ZKCL_LINUX_IMPL(set_robust_list)(struct zkcl_linux_robust_list_head *head,
					       size_t len)
{
	ARG_UNUSED(head);

	return (len == sizeof(*head)) ? 0 : -EINVAL;
}

ZKCL_LINUX_IMPL(sched_yield)(void)
{
	k_yield();

	return 0;
}

static void limit_of(unsigned int resource, struct zkcl_linux_rlimit64 *lim)
{
	struct zkcl_linux_process *p = zkcl_linux_current();

	lim->rlim_cur = RLIM_INFINITY;
	lim->rlim_max = RLIM_INFINITY;
	switch (resource) {
	case RLIMIT_STACK:
		if (p != NULL) {
			lim->rlim_cur = p->stack_size;
			lim->rlim_max = p->stack_size;
		}
		break;
	case RLIMIT_NOFILE:
		lim->rlim_cur = CONFIG_ZVFS_OPEN_MAX;
		lim->rlim_max = CONFIG_ZVFS_OPEN_MAX;
		break;
	default:
		break;
	}
}

ZKCL_LINUX_IMPL(prlimit64)(zkcl_linux_pid_t pid, unsigned int resource,
					 const struct zkcl_linux_rlimit64 *new_rlim,
					 struct zkcl_linux_rlimit64 *old_rlim)
{
	struct zkcl_linux_rlimit64 lim;

	if ((pid != 0) && (pid != sys_process_id(k_getpid()))) {
		return -ESRCH;
	}
	if (resource >= RLIM_NLIMITS) {
		return -EINVAL;
	}
	if (old_rlim != NULL) {
		if (zkcl_linux_user_ok(old_rlim, sizeof(*old_rlim), true) != 0) {
			return -EFAULT;
		}
		limit_of(resource, &lim);
		memcpy(old_rlim, &lim, sizeof(lim));
	}
	if ((new_rlim != NULL) && (zkcl_linux_user_ok(new_rlim, sizeof(*new_rlim), false) != 0)) {
		return -EFAULT;
	}
	/* limits are fixed: raising or lowering them is accepted and has no effect */

	return 0;
}

ZKCL_LINUX_IMPL(getrlimit)(unsigned int resource, struct zkcl_linux_rlimit *rlim)
{
	struct zkcl_linux_rlimit64 lim;
	struct zkcl_linux_rlimit out;

	if (resource >= RLIM_NLIMITS) {
		return -EINVAL;
	}
	if (zkcl_linux_user_ok(rlim, sizeof(*rlim), true) != 0) {
		return -EFAULT;
	}
	limit_of(resource, &lim);
	out.rlim_cur = (unsigned long)lim.rlim_cur;
	out.rlim_max = (unsigned long)lim.rlim_max;
	memcpy(rlim, &out, sizeof(out));

	return 0;
}

ZKCL_LINUX_IMPL(uname)(struct zkcl_linux_new_utsname *name)
{
	struct zkcl_linux_new_utsname uts = {
		.sysname = "Linux",
		.nodename = "zephyr",
		.release = "6.1.0-zephyr",
		.version = "#1 Zephyr " KERNEL_VERSION_STRING,
#if defined(CONFIG_X86_64)
		.machine = "x86_64",
#elif defined(CONFIG_X86)
		.machine = "i686",
#elif defined(CONFIG_ARM64)
		.machine = "aarch64",
#elif defined(CONFIG_ARM)
		.machine = "armv7l",
#elif defined(CONFIG_RISCV) && defined(CONFIG_64BIT)
		.machine = "riscv64",
#elif defined(CONFIG_RISCV)
		.machine = "riscv32",
#endif
		.domainname = "(none)",
	};

	if (zkcl_linux_user_ok(name, sizeof(*name), true) != 0) {
		return -EFAULT;
	}
	memcpy(name, &uts, sizeof(uts));

	return 0;
}

/*
 * Signal dispositions and masks are accepted and kept nowhere: nothing
 * delivers signals into a Linux process yet, so an executable that installs
 * handlers runs as if none arrive.
 */
/*
 * Signals 1..31 have the same numbers and default actions as the kernel's;
 * the guest's real-time signals and the kernel's own do not meet. Only the
 * default and ignore dispositions reach the kernel: a handler installed by
 * the guest is never called, so its signal acts as if it were not there.
 */
#define LINUX_SIG_MAX 31
#define LINUX_SIG_DFL ((zkcl_linux_sighandler_t)0)
#define LINUX_SIG_IGN ((zkcl_linux_sighandler_t)1)

static bool sig_shared(int sig)
{
	return (sig >= 1) && (sig <= LINUX_SIG_MAX) && (sig < SIGNAL_SET_SIZE) &&
	       (sig != K_SIG_CANCEL);
}

ZKCL_LINUX_IMPL(rt_sigaction)(int sig, const struct zkcl_linux_sigaction *act,
					    struct zkcl_linux_sigaction *oact, size_t sigsetsize)
{
	struct k_sig_action kact = {0};
	struct k_sig_action kold = {0};
	int ret;

	if ((sig < 1) || (sig > 64) || (sigsetsize != sizeof(zkcl_linux_sigset_t))) {
		return -EINVAL;
	}
	if ((act != NULL) && (zkcl_linux_user_ok(act, sizeof(*act), false) != 0)) {
		return -EFAULT;
	}
	if ((oact != NULL) && (zkcl_linux_user_ok(oact, sizeof(*oact), true) != 0)) {
		return -EFAULT;
	}
	if (!sig_shared(sig)) {
		if (oact != NULL) {
			memset(oact, 0, sizeof(*oact));
		}
		return 0;
	}
	if (act != NULL) {
		kact.handler = (act->sa_handler == LINUX_SIG_IGN) ? K_SIG_IGN : K_SIG_DFL;
	}
	ret = k_sig_action(sig, (act != NULL) ? &kact : NULL, &kold);
	if (ret != 0) {
		return ret;
	}
	if (oact != NULL) {
		memset(oact, 0, sizeof(*oact));
		oact->sa_handler = (kold.handler == K_SIG_IGN) ? LINUX_SIG_IGN : LINUX_SIG_DFL;
	}

	return 0;
}

ZKCL_LINUX_IMPL(rt_sigprocmask)(int how, zkcl_linux_sigset_t *set,
					      zkcl_linux_sigset_t *oset, size_t sigsetsize)
{
	struct k_sig_set cur;
	struct k_sig_set next;
	int ret;

	if ((how < 0) || (how > 2) || (sigsetsize != sizeof(*set))) {
		return -EINVAL;
	}
	if ((set != NULL) && (zkcl_linux_user_ok(set, sizeof(*set), false) != 0)) {
		return -EFAULT;
	}
	if ((oset != NULL) && (zkcl_linux_user_ok(oset, sizeof(*oset), true) != 0)) {
		return -EFAULT;
	}
	ret = k_sig_mask(K_SIG_BLOCK, NULL, &cur);
	if (ret != 0) {
		return ret;
	}
	if (oset != NULL) {
		memset(oset, 0, sizeof(*oset));
		for (int sig = 1; sig <= LINUX_SIG_MAX; sig++) {
			if (sig_shared(sig) && ((cur.sig[0] & BIT(sig)) != 0UL)) {
				oset->sig[0] |= BIT(sig - 1);
			}
		}
	}
	if (set == NULL) {
		return 0;
	}
	next = cur;
	for (int sig = 1; sig <= LINUX_SIG_MAX; sig++) {
		bool want = (set->sig[0] & BIT(sig - 1)) != 0UL;

		if (!sig_shared(sig)) {
			continue;
		}
		switch (how) {
		case 0: /* SIG_BLOCK */
			next.sig[0] |= want ? BIT(sig) : 0UL;
			break;
		case 1: /* SIG_UNBLOCK */
			next.sig[0] &= want ? ~BIT(sig) : ~0UL;
			break;
		default: /* SIG_SETMASK */
			next.sig[0] = want ? (next.sig[0] | BIT(sig)) : (next.sig[0] & ~BIT(sig));
			break;
		}
	}

	return k_sig_mask(K_SIG_SETMASK, &next, NULL);
}

#define LINUX_WNOHANG 1

ZKCL_LINUX_IMPL(wait4)(zkcl_linux_pid_t upid, int *stat_addr, int options,
				     struct zkcl_linux_rusage *ru)
{
	k_pid_t pid = NULL;
	k_pid_t reaped = NULL;
	uint32_t opts = 0;
	int status = 0;
	int ret;

	if (upid > 0) {
		pid = sys_process_find(upid);
		if (pid == NULL) {
			return -ECHILD;
		}
	} else if (upid != -1) {
		/* process groups are not selectable yet */
		return -EINVAL;
	}
	if ((stat_addr != NULL) && (zkcl_linux_user_ok(stat_addr, sizeof(*stat_addr), true) != 0)) {
		return -EFAULT;
	}
	if ((options & LINUX_WNOHANG) != 0) {
		opts |= K_PROCESS_WNOHANG;
	}
	while (true) {
		/* the number retires with the reap: peek, note it, then reap that child */
		ret = k_waitpid(pid, &reaped, &status, opts | K_PROCESS_WNOWAIT, K_FOREVER);
		if (ret == -EAGAIN) {
			return 0;
		}
		if (ret < 0) {
			return ret;
		}
		upid = sys_process_id(reaped);
		ret = k_waitpid(reaped, NULL, NULL, K_PROCESS_WNOHANG, K_NO_WAIT);
		if (ret == 0) {
			break;
		}
		/* another waiter took it, or the handle already names a new process */
	}
	if (stat_addr != NULL) {
		/* the kernel's wait status is Linux-valued */
		memcpy(stat_addr, &status, sizeof(status));
	}
	ARG_UNUSED(ru);

	return upid;
}

static long kill_impl(zkcl_linux_pid_t pid, int sig)
{
	k_pid_t target;

	if (pid <= 0) {
		return -EINVAL;
	}
	target = sys_process_find(pid);
	if (target == NULL) {
		return -ESRCH;
	}

	return k_kill(target, sig);
}

ZKCL_LINUX_IMPL(kill)(zkcl_linux_pid_t pid, int sig)
{
	return kill_impl(pid, sig);
}

ZKCL_LINUX_IMPL(tgkill)(zkcl_linux_pid_t tgid, zkcl_linux_pid_t pid, int sig)
{
	/* a process has one thread, whose id is the process id */
	if ((tgid <= 0) || (pid != tgid)) {
		return (tgid <= 0) ? -EINVAL : -ESRCH;
	}

	return kill_impl(pid, sig);
}

ZKCL_LINUX_IMPL(tkill)(zkcl_linux_pid_t pid, int sig)
{
	return kill_impl(pid, sig);
}

ZKCL_LINUX_IMPL(sched_getaffinity)(zkcl_linux_pid_t pid, unsigned int len,
						 unsigned long *user_mask_ptr)
{
	unsigned long mask = BIT_MASK(CONFIG_MP_MAX_NUM_CPUS);

	ARG_UNUSED(pid);
	if (len < sizeof(mask)) {
		return -EINVAL;
	}
	if (zkcl_linux_user_ok(user_mask_ptr, sizeof(mask), true) != 0) {
		return -EFAULT;
	}
	memcpy(user_mask_ptr, &mask, sizeof(mask));

	return sizeof(mask);
}

ZKCL_LINUX_IMPL(umask)(int mask)
{
	ARG_UNUSED(mask);

	return 022;
}

ZKCL_LINUX_IMPL(getpgrp)(void)
{
	return sys_pgrp_id(k_getpgid(k_getpid()));
}

/*
 * fork(): the process layer copies the address space and resumes the child
 * from this very system call with a zero result; the parent starts it once
 * the child owns the layer's state and the thread pointer.
 */
static long linux_fork(void)
{
	struct sys_clone_args args = {
		.flags = SYS_CLONE_VM_COPY | SYS_CLONE_PAUSED,
	};
	k_pid_t child;
	int ret;

	if (!IS_ENABLED(CONFIG_PROCESS_VM)) {
		return -ENOSYS;
	}
	ret = sys_clone(&args, &child);
	if (ret < 0) {
		return (ret == -ENOTSUP) ? -ENOSYS : ret;
	}
	ret = zkcl_linux_process_clone(child);
	if (ret < 0) {
		k_thread_abort(child);
		(void)k_waitpid(child, NULL, NULL, 0, K_FOREVER);
		return ret;
	}
	k_thread_start(child);

	return sys_process_id(child);
}

#define CSIGNAL 0x000000ff
#define CLONE_PARENT_SETTID 0x00100000
#define CLONE_CHILD_CLEARTID 0x00200000
#define CLONE_CHILD_SETTID 0x01000000
#define CLONE_FORK_FLAGS (CSIGNAL | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID | CLONE_CHILD_SETTID)

ZKCL_LINUX_IMPL(fork)(void)
{
	return linux_fork();
}

ZKCL_LINUX_IMPL(vfork)(void)
{
	/* a full copy: the child cannot share the parent's memory */
	return linux_fork();
}

/* only the fork() flavour: a new stack or a shared address space is a thread */
ZKCL_LINUX_IMPL(clone)(unsigned long arg1, unsigned long arg2, int *arg3,
				     unsigned long arg4, int *arg5)
{
	unsigned long flags = arg1;
	unsigned long newsp = arg2;

	ARG_UNUSED(arg3);
	ARG_UNUSED(arg4);
	ARG_UNUSED(arg5);
	if ((newsp != 0UL) || ((flags & ~CLONE_FORK_FLAGS) != 0UL)) {
		return -ENOSYS;
	}
	/* the tid pointers are not written: the child's copies are its own */

	return linux_fork();
}

ZKCL_LINUX_IMPL(getgroups)(int gidsetsize, zkcl_linux_gid_t *grouplist)
{
	ARG_UNUSED(grouplist);
	if (gidsetsize < 0) {
		return -EINVAL;
	}
	/* root, in no supplementary group */

	return 0;
}

#define EXEC_VECTOR_MAX 64

/* a vector of the executable's: every pointer and string readable */
static int exec_vector_ok(const char *const *vec)
{
	if (vec == NULL) {
		return 0;
	}
	for (size_t i = 0; i <= EXEC_VECTOR_MAX; i++) {
		const char *s;
		size_t len;

		if (zkcl_linux_user_ok(&vec[i], sizeof(*vec), false) != 0) {
			return -EFAULT;
		}
		s = vec[i];
		if (s == NULL) {
			return 0;
		}
		for (len = 0; len < CONFIG_SYS_PROCESS_ARG_BYTES; len++) {
			if ((((uintptr_t)s + len) & (ZKCL_LINUX_PAGE_SIZE - 1)) == 0 || (len == 0)) {
				size_t left = ZKCL_LINUX_PAGE_SIZE -
					      (((uintptr_t)s + len) & (ZKCL_LINUX_PAGE_SIZE - 1));

				if (zkcl_linux_user_ok(s + len, left, false) != 0) {
					return -EFAULT;
				}
			}
			if (s[len] == '\0') {
				break;
			}
		}
		if (len == CONFIG_SYS_PROCESS_ARG_BYTES) {
			return -E2BIG;
		}
	}

	return -E2BIG;
}

/*
 * The image is replaced in place, then the leader restarts through the
 * process layer's exec with the vectors staged from the old image's memory,
 * which stays readable to the kernel until the next load frees it.
 */
/* argv and envp, validated, packed into one kernel block: pointers, then strings */
static int exec_vectors_pack(const char *const *argv, const char *const *envp, char ***kargv,
			     char ***kenvp, void **block)
{
	const char *const *vecs[2] = {argv, envp};
	size_t counts[2] = {0, 0};
	size_t bytes = 0;
	char **ptrs;
	char *strs;

	for (int v = 0; v < 2; v++) {
		for (size_t i = 0; (vecs[v] != NULL) && (vecs[v][i] != NULL); i++) {
			bytes += strlen(vecs[v][i]) + 1;
			counts[v]++;
		}
	}
	*block = k_malloc(((counts[0] + counts[1] + 2) * sizeof(char *)) + bytes);
	if (*block == NULL) {
		return -ENOMEM;
	}
	ptrs = *block;
	strs = (char *)&ptrs[counts[0] + counts[1] + 2];
	*kargv = ptrs;
	*kenvp = &ptrs[counts[0] + 1];
	for (int v = 0; v < 2; v++) {
		for (size_t i = 0; i < counts[v]; i++) {
			size_t len = strlen(vecs[v][i]) + 1;

			memcpy(strs, vecs[v][i], len);
			*ptrs++ = strs;
			strs += len;
		}
		*ptrs++ = NULL;
	}

	return 0;
}

ZKCL_LINUX_IMPL(execve)(const char *filename, const char *const *argv,
				      const char *const *envp)
{
	char path[256];
	char **kargv;
	char **kenvp;
	void *block;
	int ret;

	ret = zkcl_linux_user_string(path, filename, sizeof(path));
	if (ret != 0) {
		return ret;
	}
	ret = exec_vector_ok(argv);
	if (ret == 0) {
		ret = exec_vector_ok(envp);
	}
	if (ret != 0) {
		return ret;
	}
	/* the old image's memory leaves the address space before the new one starts */
	ret = exec_vectors_pack(argv, envp, &kargv, &kenvp, &block);
	if (ret != 0) {
		return ret;
	}
	ret = zkcl_linux_exec_self(path);
	if (ret != 0) {
		k_free(block);
		return ret;
	}
	ret = sys_exec_start(zkcl_linux_start, kargv, kenvp);
	/* the old image is gone: nothing to return to */
	k_free(block);
	exit_process(127);
	CODE_UNREACHABLE;
}

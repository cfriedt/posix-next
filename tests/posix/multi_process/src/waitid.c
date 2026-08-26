/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>
#include <zephyr/ztest.h>

#ifndef CONFIG_USERSPACE
#define WAITID_CHILD_STACK_SIZE 1024

static K_THREAD_STACK_DEFINE(waitid_child_stack, WAITID_CHILD_STACK_SIZE);
static struct k_thread waitid_child_thread;

static void waitid_child_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (p1 != NULL) {
		/* linger so the parent can observe "no state change" first */
		k_sleep(K_MSEC(100));
	}
	_exit(7);
}

static pid_t waitid_spawn(bool linger)
{
	k_pid_t child = NULL;
	struct sys_clone_args args = {
		.entry = waitid_child_entry,
		.p1 = linger ? (void *)1 : NULL,
		.thread = &waitid_child_thread,
		.stack = waitid_child_stack,
		.stack_size = WAITID_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};

	zassert_ok(sys_clone(&args, &child));

	return (pid_t)sys_process_id(child);
}

/* P_PID: WNOWAIT observes without reaping, and the reap fills siginfo */
static void waitid_pid_nowait_reap(void)
{
	siginfo_t info;
	pid_t pid = waitid_spawn(false);

	zassert_true(pid > 0);

	info = (siginfo_t){0};
	zassert_ok(waitid(P_PID, (id_t)pid, &info, WEXITED | WNOWAIT));
	zassert_equal(info.si_signo, SIGCHLD);
	zassert_equal(info.si_pid, pid);
	zassert_equal(info.si_code, CLD_EXITED);
	zassert_equal(info.si_status, 7);

	/* WNOWAIT left the child reapable */
	info = (siginfo_t){0};
	zassert_ok(waitid(P_PID, (id_t)pid, &info, WEXITED));
	zassert_equal(info.si_pid, pid);
	zassert_equal(info.si_status, 7);
}

/* P_ALL: WNOHANG reports "no state change" with si_pid 0, then blocks to reap */
static void waitid_all_nohang(void)
{
	siginfo_t info;
	pid_t pid = waitid_spawn(true);

	zassert_true(pid > 0);

	info = (siginfo_t){.si_pid = -1};
	zassert_ok(waitid(P_ALL, 0, &info, WEXITED | WNOHANG));
	zassert_equal(info.si_pid, 0, "WNOHANG with a live child did not leave si_pid 0");

	info = (siginfo_t){0};
	zassert_ok(waitid(P_ALL, 0, &info, WEXITED));
	zassert_equal(info.si_pid, pid);
	zassert_equal(info.si_status, 7);
}

/* P_PGID: both the caller's own group (id 0) and the group named explicitly */
static void waitid_pgid(void)
{
	siginfo_t info;
	pid_t pid = waitid_spawn(false);

	zassert_true(pid > 0);

	info = (siginfo_t){0};
	zassert_ok(waitid(P_PGID, (id_t)getpgrp(), &info, WEXITED));
	zassert_equal(info.si_pid, pid);
	zassert_equal(info.si_status, 7);

	pid = waitid_spawn(false);
	zassert_true(pid > 0);

	info = (siginfo_t){0};
	zassert_ok(waitid(P_PGID, 0, &info, WEXITED));
	zassert_equal(info.si_pid, pid);
}
#endif /* !CONFIG_USERSPACE */

ZTEST_USER(posix_multi_process, test_waitid)
{
	siginfo_t info;

	errno = 0;
	zexpect_equal(waitid(P_ALL, 0, &info, WEXITED), -1);
	zexpect_equal(errno, ECHILD);

	errno = 0;
	zexpect_equal(waitid(P_PID, 1, &info, WEXITED | WNOHANG | WNOWAIT), -1);
	zexpect_equal(errno, ECHILD);

	errno = 0;
	zexpect_equal(waitid((idtype_t)0xdead, 0, &info, WEXITED), -1);
	zexpect_equal(errno, EINVAL);

	/* an option outside the waitid() set is invalid */
	errno = 0;
	zexpect_equal(waitid(P_ALL, 0, &info, WEXITED | 0x10000), -1);
	zexpect_equal(errno, EINVAL);

	/* a group that does not exist has no children to wait for */
	errno = 0;
	zexpect_equal(waitid(P_PGID, 0x7ff0, &info, WEXITED), -1);
	zexpect_equal(errno, ECHILD);

#ifndef CONFIG_USERSPACE
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && !k_is_user_context()) {
		waitid_pid_nowait_reap();
		waitid_all_nohang();
		waitid_pgid();
	}
#endif /* !CONFIG_USERSPACE */
}

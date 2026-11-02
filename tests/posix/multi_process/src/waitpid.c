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
#define WAITPID_CHILD_STACK_SIZE 1024

static K_THREAD_STACK_DEFINE(waitpid_child_stack, WAITPID_CHILD_STACK_SIZE);
static struct k_thread waitpid_child_thread;

static void waitpid_child_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (p1 != NULL) {
		/* linger so the parent can observe "no state change" first, or be interrupted */
		k_sleep(K_MSEC(p1 == (void *)2 ? 2000 : 100));
	}
	_exit(5);
}

static pid_t waitpid_spawn(int linger)
{
	k_pid_t child = NULL;
	struct sys_clone_args args = {
		.entry = waitpid_child_entry,
		.p1 = (void *)(uintptr_t)linger,
		.thread = &waitpid_child_thread,
		.stack = waitpid_child_stack,
		.stack_size = WAITPID_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};

	zassert_ok(sys_clone(&args, &child));

	return (pid_t)sys_process_id(child);
}

/* re-armed: the first alarm may land before the wait it is meant to interrupt */
static void waitpid_signal_handler(int sig)
{
	ARG_UNUSED(sig);
	(void)alarm(1);
}

/* a caught signal ends the wait; the child is reaped afterwards */
static void waitpid_interrupted(void)
{
	struct sigaction act = {.sa_handler = waitpid_signal_handler};
	sigset_t set;
	pid_t pid = waitpid_spawn(2);

	zassert_true(pid > 0);
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGALRM));
	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));
	zassert_ok(sigaction(SIGALRM, &act, NULL));
	zassert_equal(alarm(1), 0);
	zassert_equal(waitpid(pid, NULL, 0), -1);
	zassert_equal(errno, EINTR);
	zassert_true(alarm(0) <= 1);
	act.sa_handler = SIG_DFL;
	zassert_ok(sigaction(SIGALRM, &act, NULL));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));

	zassert_equal(waitpid(pid, NULL, 0), pid);
}

static volatile pid_t waitpid_chld_pid;

static void waitpid_chld_handler(int sig, siginfo_t *info, void *ucontext)
{
	ARG_UNUSED(sig);
	ARG_UNUSED(ucontext);

	waitpid_chld_pid = info->si_pid;
}

/* the reap does not discard the pending SIGCHLD: the handler runs after waitpid() */
static void waitpid_sigchld(void)
{
	struct sigaction act = {.sa_sigaction = waitpid_chld_handler, .sa_flags = SA_SIGINFO};
	sigset_t set;
	pid_t pid;

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGCHLD));
	zassert_ok(sigaction(SIGCHLD, &act, NULL));
	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));
	waitpid_chld_pid = 0;

	pid = waitpid_spawn(false);
	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, NULL, 0), pid);
	/* a delivery point for a kernel-mode caller */
	k_msleep(1);
	zassert_equal(waitpid_chld_pid, pid, "the reap discarded the pending SIGCHLD (%d vs %d)",
		      (int)waitpid_chld_pid, (int)pid);

	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	zassert_ok(sigaction(SIGCHLD, &act, NULL));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
}

#ifdef CONFIG_POSIX_JOB_CONTROL
static void waitpid_stop_child_entry(void *p1, void *p2, void *p3)
{
	sigset_t set;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	(void)sigemptyset(&set);
	(void)sigaddset(&set, SIGSTOP);
	(void)sigaddset(&set, SIGCONT);
	(void)sigprocmask(SIG_UNBLOCK, &set, NULL);
	/* delivery points */
	for (int i = 0; i < 200; i++) {
		k_sleep(K_MSEC(10));
	}
	_exit(7);
}

/* a SIGCHLD handler does not hide a stop or a continue from the wait that it interrupts */
static void waitpid_stopped_handled(void)
{
	struct sigaction act = {.sa_sigaction = waitpid_chld_handler, .sa_flags = SA_SIGINFO};
	struct sys_clone_args args = {
		.entry = waitpid_stop_child_entry,
		.thread = &waitpid_child_thread,
		.stack = waitpid_child_stack,
		.stack_size = WAITPID_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};
	k_pid_t child = NULL;
	sigset_t set;
	int status = 0;
	pid_t pid;

	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGCHLD));
	zassert_ok(sigaction(SIGCHLD, &act, NULL));
	zassert_ok(sigprocmask(SIG_UNBLOCK, &set, NULL));

	zassert_ok(sys_clone(&args, &child));
	pid = (pid_t)sys_process_id(child);
	zassert_ok(kill(pid, SIGSTOP));
	zassert_equal(waitpid(pid, &status, WUNTRACED), pid, "errno %d", errno);
	zassert_true(WIFSTOPPED(status), "0x%x", status);
	zassert_ok(kill(pid, SIGCONT));
	zassert_equal(waitpid(pid, &status, WCONTINUED), pid, "errno %d", errno);
	zassert_true(WIFCONTINUED(status), "0x%x", status);
	zassert_ok(kill(pid, SIGKILL));
	zassert_equal(waitpid(pid, &status, 0), pid, "errno %d", errno);
	zassert_true(WIFSIGNALED(status) && (WTERMSIG(status) == SIGKILL), "0x%x", status);

	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	zassert_ok(sigaction(SIGCHLD, &act, NULL));
	zassert_ok(sigprocmask(SIG_BLOCK, &set, NULL));
}
#endif /* CONFIG_POSIX_JOB_CONTROL */

/* pid > 0: a blocking wait fills the status; a NULL stat_loc is allowed */
static void waitpid_specific(void)
{
	int status = -1;
	pid_t pid = waitpid_spawn(false);

	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 5);

	pid = waitpid_spawn(false);
	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, NULL, 0), pid);
}

/* pid -1: WNOHANG returns 0 while the child lives, then a blocking reap */
static void waitpid_any_nohang(void)
{
	int status = -1;
	pid_t pid = waitpid_spawn(true);

	zassert_true(pid > 0);
	zassert_equal(waitpid(-1, &status, WNOHANG), 0,
		      "WNOHANG with a live child did not return 0");
	zassert_equal(waitpid(-1, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 5);
}

/* pid 0 and pid < -1 select the caller's and the named process group */
static void waitpid_pgrp(void)
{
	int status = -1;
	pid_t pid = waitpid_spawn(false);

	zassert_true(pid > 0);
	zassert_equal(waitpid(0, &status, 0), pid);
	zassert_true(WIFEXITED(status));

	pid = waitpid_spawn(false);
	zassert_true(pid > 0);
	zassert_equal(waitpid(-getpgrp(), &status, 0), pid);
	zassert_equal(WEXITSTATUS(status), 5);
}
#endif /* !CONFIG_USERSPACE */

ZTEST_USER(posix_multi_process, test_waitpid)
{
	int status;

	errno = 0;
	zexpect_equal(waitpid(-1, &status, 0), -1);
	zexpect_equal(errno, ECHILD);

	errno = 0;
	zexpect_equal(waitpid(-1, &status, WNOHANG), -1);
	zexpect_equal(errno, ECHILD);

	/* WNOWAIT is a waitid()-only option; waitpid() rejects it */
	errno = 0;
	zexpect_equal(waitpid(-1, &status, WNOWAIT), -1);
	zexpect_equal(errno, EINVAL);

	/* a pid that is not a child of the caller */
	errno = 0;
	zexpect_equal(waitpid(1, &status, 0), -1);
	zexpect_equal(errno, ECHILD);

	/* a group that does not exist has no children to wait for */
	errno = 0;
	zexpect_equal(waitpid(-0x7ff0, &status, 0), -1);
	zexpect_equal(errno, ECHILD);

#ifndef CONFIG_USERSPACE
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && !k_is_user_context()) {
		waitpid_specific();
		waitpid_any_nohang();
		waitpid_pgrp();
		waitpid_interrupted();
		waitpid_sigchld();
#ifdef CONFIG_POSIX_JOB_CONTROL
		waitpid_stopped_handled();
#endif /* CONFIG_POSIX_JOB_CONTROL */
	}
#endif /* !CONFIG_USERSPACE */
}

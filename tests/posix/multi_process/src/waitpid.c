/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
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
		/* linger so the parent can observe "no state change" first */
		k_sleep(K_MSEC(100));
	}
	_exit(5);
}

static pid_t waitpid_spawn(bool linger)
{
	k_pid_t child = NULL;
	struct sys_clone_args args = {
		.entry = waitpid_child_entry,
		.p1 = linger ? (void *)1 : NULL,
		.thread = &waitpid_child_thread,
		.stack = waitpid_child_stack,
		.stack_size = WAITPID_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};

	zassert_ok(sys_clone(&args, &child));

	return (pid_t)sys_process_id(child);
}

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
	}
#endif /* !CONFIG_USERSPACE */
}

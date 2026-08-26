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
#define SETPGID_CHILD_STACK_SIZE 1024

static K_THREAD_STACK_DEFINE(setpgid_child_stack, SETPGID_CHILD_STACK_SIZE);
static struct k_thread setpgid_child_thread;

static void setpgid_child_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	_exit(0);
}

/*
 * The positive paths need a target that is not a session leader: a paused
 * child, whose group the parent may set until it execs (POSIX).
 */
static void setpgid_child_group(void)
{
	int status = -1;
	k_pid_t child = NULL;
	pid_t cpid;
	struct sys_clone_args args = {
		.flags = SYS_CLONE_PAUSED,
		.entry = setpgid_child_entry,
		.thread = &setpgid_child_thread,
		.stack = setpgid_child_stack,
		.stack_size = SETPGID_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};

	zassert_ok(sys_clone(&args, &child));
	cpid = (pid_t)sys_process_id(child);
	zassert_true(cpid > 0);

	/* the child starts in its parent's group */
	zassert_equal(getpgid(cpid), getpgrp());

	/* pgid == pid creates a new group led by the target */
	zassert_ok(setpgid(cpid, cpid));
	zassert_equal(getpgid(cpid), cpid);

	/* any other group must already exist */
	errno = 0;
	zexpect_equal(setpgid(cpid, cpid + 1), -1);
	zexpect_equal(errno, EPERM);

	/* and an existing group in the same session may be joined */
	zassert_ok(setpgid(cpid, getpgrp()));
	zassert_equal(getpgid(cpid), getpgrp());

	k_thread_start((k_tid_t)child);
	zassert_equal(waitpid(cpid, &status, 0), cpid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 0);
}
#endif /* !CONFIG_USERSPACE */

ZTEST_USER(posix_multi_process, test_setpgid)
{
	/* a negative process or group id is invalid */
	errno = 0;
	zexpect_equal(setpgid(-1, 0), -1);
	zexpect_equal(errno, EINVAL);

	errno = 0;
	zexpect_equal(setpgid(0, -1), -1);
	zexpect_equal(errno, EINVAL);

	if (!IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		pid_t self = getpid();

		/* a nonexistent pid is ESRCH; on the host self + 1 may be a live process */
		errno = 0;
		zexpect_equal(setpgid(self + 1, 0), -1);
		zexpect_equal(errno, ESRCH);

		/* pid 0 names the caller; a nonexistent foreign group is EPERM */
		errno = 0;
		zexpect_equal(setpgid(0, self + 1), -1);
		zexpect_equal(errno, EPERM);
	}

#ifndef CONFIG_USERSPACE
	if (!IS_ENABLED(CONFIG_NATIVE_LIBC) && !k_is_user_context()) {
		setpgid_child_group();
	}
#endif /* !CONFIG_USERSPACE */
}

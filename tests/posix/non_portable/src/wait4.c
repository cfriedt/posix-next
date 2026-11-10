/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>
#include <zephyr/ztest.h>

#if defined(CONFIG_POSIX_MULTI_PROCESS) && !defined(CONFIG_USERSPACE)
#define WAIT4_CHILD_STACK_SIZE 1024

static K_THREAD_STACK_DEFINE(wait4_child_stack, WAIT4_CHILD_STACK_SIZE);
static struct k_thread wait4_child_thread;

static void wait4_child_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	/* some CPU time of its own to report */
	k_busy_wait(3000);
	_exit((int)(uintptr_t)p1);
}

static pid_t wait4_spawn(int code)
{
	k_pid_t child = NULL;
	struct sys_clone_args args = {
		.entry = wait4_child_entry,
		.p1 = (void *)(uintptr_t)code,
		.thread = &wait4_child_thread,
		.stack = wait4_child_stack,
		.stack_size = WAIT4_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};

	zassert_ok(sys_clone(&args, &child));

	return (pid_t)sys_process_id(child);
}

static void usage_checks(const struct rusage *ru)
{
	if (IS_ENABLED(CONFIG_SCHED_THREAD_USAGE)) {
		zassert_true((ru->ru_utime.tv_sec > 0) || (ru->ru_utime.tv_usec > 0),
			     "the child's CPU time is reported as user time");
	}
	/* one CPU time per thread: none of it is system time */
	zassert_equal(ru->ru_stime.tv_sec, 0);
	zassert_equal(ru->ru_stime.tv_usec, 0);
}

ZTEST(posix_non_portable, test_wait4)
{
	struct rusage ru = {.ru_stime.tv_sec = 1};
	int status = 0;
	pid_t pid;

	/* a specific child, with its usage */
	pid = wait4_spawn(3);
	zassert_equal(wait4(pid, &status, 0, &ru), pid, "errno %d", errno);
	zassert_true(WIFEXITED(status) && (WEXITSTATUS(status) == 3), "0x%x", status);
	usage_checks(&ru);

	/* any child; a NULL status and a NULL rusage are allowed */
	pid = wait4_spawn(5);
	zassert_equal(wait4(-1, NULL, 0, NULL), pid, "errno %d", errno);

	/* the options are waitpid()'s */
	zassert_equal(wait4(-1, &status, ~0, &ru), -1);
	zassert_equal(errno, EINVAL);
	/* no child left */
	zassert_equal(wait4(-1, &status, WNOHANG, &ru), -1);
	zassert_equal(errno, ECHILD);
}

ZTEST(posix_non_portable, test_wait3)
{
	struct rusage ru = {.ru_stime.tv_sec = 1};
	int status = 0;
	pid_t pid;

	pid = wait4_spawn(7);
	zassert_equal(wait3(&status, 0, &ru), pid, "errno %d", errno);
	zassert_true(WIFEXITED(status) && (WEXITSTATUS(status) == 7), "0x%x", status);
	usage_checks(&ru);
	zassert_equal(wait3(&status, WNOHANG, NULL), -1);
	zassert_equal(errno, ECHILD);
}
#else
ZTEST(posix_non_portable, test_wait4)
{
	ztest_test_skip();
}

ZTEST(posix_non_portable, test_wait3)
{
	ztest_test_skip();
}
#endif /* CONFIG_POSIX_MULTI_PROCESS && !CONFIG_USERSPACE */

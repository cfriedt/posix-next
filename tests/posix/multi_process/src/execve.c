/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/process.h>
#include <zephyr/ztest.h>

/*
 * Prelinked-image resolution walks a registry in kernel memory, so it only
 * works from supervisor mode until an exec loader exists (M3). The in-place
 * exec round trip below is therefore supervisor-only; a user-mode execve()
 * resolves nothing and returns ENOENT.
 */
#ifndef CONFIG_USERSPACE
#include "image_registry.h"

static char *const exec_argv[] = {"execme", NULL};
static volatile int exec_keep_fd = -1;
static volatile int exec_close_fd = -1;

static void execme_entry(void *p1, void *p2, void *p3)
{
	char *const *argv = p1;
	bool argv_ok;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* the previous image's argv arrives by value: exec copies the vectors */
	argv_ok = (argv != NULL) && (argv[0] != NULL) && (strcmp(argv[0], "execme") == 0) &&
		  (argv[1] == NULL);
	if (!argv_ok) {
		_exit(43);
	}
	/* FD_CLOEXEC closed across exec; the plain descriptor survived */
	if ((fcntl(exec_close_fd, F_GETFD) != -1) || (errno != EBADF)) {
		_exit(44);
	}
	if (eventfd_write(exec_keep_fd, 1) != 0) {
		_exit(45);
	}
	_exit(42);
}

IMAGE_REGISTRY_ENTRY_DEFINE(img_execme, "/bin/execme", execme_entry);

static K_THREAD_STACK_DEFINE(exec_child_stack, 2048);
static struct k_thread exec_child_thread;

static void exec_child_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	exec_keep_fd = eventfd(0, 0);
	exec_close_fd = eventfd(0, 0);
	if ((exec_keep_fd < 0) || (exec_close_fd < 0) ||
	    (fcntl(exec_close_fd, F_SETFD, FD_CLOEXEC) != 0)) {
		_exit(98);
	}

	(void)execve("/bin/execme", exec_argv, NULL);
	_exit(99);
}
#endif /* !CONFIG_USERSPACE */

ZTEST_USER(posix_multi_process, test_execve)
{
	if (IS_ENABLED(CONFIG_NATIVE_LIBC)) {
		/* host libc actually execs/forks; the prelinked-image resolution is Zephyr-only */
		ztest_test_skip();
		return;
	}

	char *const argv[] = {"true", NULL};
	char *const envp[] = {NULL};

	ARG_UNUSED(envp);

	errno = 0;
	zexpect_equal(execve("/bin/true", argv, envp), -1);
	zexpect_equal(errno, ENOENT);

#ifndef CONFIG_USERSPACE
	if (!k_is_user_context()) {
		/* a child execs a registry image in place: same pid, new image */
		int status = -1;
		k_pid_t child = NULL;
		struct sys_clone_args args = {
			.entry = exec_child_entry,
			.thread = &exec_child_thread,
			.stack = exec_child_stack,
			.stack_size = 2048,
			.prio = k_thread_priority_get(k_current_get()),
		};

		zassert_ok(sys_clone(&args, &child));

		pid_t pid = (pid_t)sys_process_id(child);

		zassert_true(pid > 0);
		zassert_equal(waitpid(pid, &status, 0), pid);
		zassert_true(WIFEXITED(status));
		zassert_equal(WEXITSTATUS(status), 42, "exec image status %d", WEXITSTATUS(status));
	}
#endif /* !CONFIG_USERSPACE */
}

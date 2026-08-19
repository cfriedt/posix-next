/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ff.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/llext/llext.h>
#include <zephyr/llext/symbol.h>
#include <zephyr/sys/process.h>
#include <zephyr/ztest.h>

/* resolve the image's _exit() reference at llext link time */
EXPORT_SYMBOL(_exit);

#define EXEC_IMAGE "/RAM:/hello.llext"

static const uint8_t hello_llext[] = {
#include "hello_ext.inc"
};

static FATFS fat_fs;
static struct fs_mount_t fs_mnt = {
	.type = FS_FATFS,
	.mnt_point = "/RAM:",
	.fs_data = &fat_fs,
};

/* execve() loads the extension on the caller's stack; 64-bit frames need room */
#define EXEC_CHILD_STACK_SIZE (IS_ENABLED(CONFIG_64BIT) ? 4096 : 2048)

static K_THREAD_STACK_DEFINE(exec_child_stack, EXEC_CHILD_STACK_SIZE);
static struct k_thread exec_child_thread;

static char *const exec_argv[] = {"hello", "x", NULL};
static char *const exec_exit_argv[] = {"hello", "e", NULL};

static void exec_child_entry(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	(void)execve(EXEC_IMAGE, p1, NULL);
	_exit(99);
}

static pid_t exec_spawn(char *const argv[])
{
	k_pid_t child = NULL;
	struct sys_clone_args args = {
		.entry = exec_child_entry,
		.p1 = (void *)argv,
		.thread = &exec_child_thread,
		.stack = exec_child_stack,
		.stack_size = EXEC_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};

	zassert_ok(sys_clone(&args, &child));

	return (pid_t)sys_process_id(child);
}

static void execve_return_status(void)
{
	int status = -1;
	pid_t pid = exec_spawn(exec_argv);

	/* a child execs a real ELF extension: same pid, freshly loaded image */
	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 42, "extension main() did not run: %d",
		      WEXITSTATUS(status));
	zassert_is_null(llext_by_name("hello.llext"), "image not unloaded after a normal return");
}

static void execve_exit_reaps_image(void)
{
	int status = -1;
	pid_t pid = exec_spawn(exec_exit_argv);

	/* an image dying via _exit() cannot unload itself; the reaper must */
	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 43, "extension _exit status lost: %d",
		      WEXITSTATUS(status));
	zassert_is_null(llext_by_name("hello.llext"), "image leaked after _exit()");
}

ZTEST(posix_exec, test_execve)
{
	execve_return_status();
	execve_exit_reaps_image();
}

static void *exec_setup(void)
{
	int fd;
	int ret;

	zassert_ok(fs_mount(&fs_mnt));
	fd = open(EXEC_IMAGE, O_RDWR | O_CREAT, 0644);
	zassert_true(fd >= 0, "open failed: %d", errno);
	ret = write(fd, hello_llext, sizeof(hello_llext));
	zassert_equal(ret, (int)sizeof(hello_llext));
	zassert_ok(close(fd));

	return NULL;
}

ZTEST_SUITE(posix_exec, NULL, exec_setup, NULL, NULL, NULL);

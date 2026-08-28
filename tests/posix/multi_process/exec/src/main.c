/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ff.h>
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>
#include <zephyr/llext/llext.h>
#include <zephyr/llext/symbol.h>
#include <zephyr/sys/process.h>
#include <zephyr/ztest.h>

/* resolve the image's _exit() and execve() references at llext link time */
EXPORT_SYMBOL(_exit);
EXPORT_SYMBOL(execve);

#define EXEC_IMAGE "/RAM:/hello.llext"
#define EXEC_CHAIN_IMAGE "/RAM:/hello2.llext"
#define EXEC_NOEXEC_IMAGE "/RAM:/noexec.llext"

static const uint8_t hello_llext[] = {
#include "hello_ext.inc"
};

static const uint8_t noexec_llext[] = {
#include "noexec_ext.inc"
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

/* enough children to occupy every image slot at once */
static K_THREAD_STACK_ARRAY_DEFINE(exec_extra_stacks, CONFIG_POSIX_EXEC_LLEXT_MAX,
				   EXEC_CHILD_STACK_SIZE);
static struct k_thread exec_extra_threads[CONFIG_POSIX_EXEC_LLEXT_MAX];

static char *const exec_argv[] = {"hello", "x", NULL};
static char *const exec_exit_argv[] = {"hello", "e", NULL};
static char *const exec_chain_argv[] = {"hello", "c", NULL};

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

/* a chain-exec'ing image is unloaded by its successor past the point of no return */
static void execve_chain_unloads_prior(void)
{
	int status = -1;
	pid_t pid = exec_spawn(exec_chain_argv);

	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 43, "chain-exec'd image did not run: %d",
		      WEXITSTATUS(status));
	zassert_is_null(llext_by_name("hello.llext"), "chain-exec leaked the prior image");
	zassert_is_null(llext_by_name("hello2.llext"), "successor image not unloaded by reap");
}

/* failed resolution and loading report errors without disturbing the caller */
static void execve_load_errors(void)
{
	char *const argv[] = {"nope", NULL};

	/* nothing prelinked or on the file system by that name */
	errno = 0;
	zassert_equal(execve("/RAM:/nope.llext", argv, NULL), -1);
	zassert_equal(errno, ENOENT);

	/* a loadable object that exports no main() is not an executable image */
	errno = 0;
	zassert_equal(execve(EXEC_NOEXEC_IMAGE, argv, NULL), -1);
	zassert_equal(errno, ENOEXEC);
	zassert_is_null(llext_by_name("noexec.llext"), "refused image left loaded");
}

/* combined argument strings past the exec budget fail after the load */
static void execve_args_too_big(void)
{
	static char big[CONFIG_POSIX_EXEC_ARG_BYTES + 1];
	char *const argv[] = {"hello", big, NULL};

	memset(big, 'a', sizeof(big) - 1);

	errno = 0;
	zassert_equal(execve(EXEC_IMAGE, argv, NULL), -1);
	zassert_equal(errno, E2BIG);
	zassert_is_null(llext_by_name("hello.llext"), "refused image left loaded");
}

/* zombies not yet waited for hold their image slots; a full table is ENOMEM */
static void execve_table_enomem(void)
{
	int status = -1;
	siginfo_t info;
	pid_t pids[CONFIG_POSIX_EXEC_LLEXT_MAX];

	for (size_t i = 0; i < ARRAY_SIZE(pids); i++) {
		k_pid_t child = NULL;
		struct sys_clone_args args = {
			.entry = exec_child_entry,
			.p1 = (void *)exec_exit_argv,
			.thread = &exec_extra_threads[i],
			.stack = exec_extra_stacks[i],
			.stack_size = EXEC_CHILD_STACK_SIZE,
			.prio = k_thread_priority_get(k_current_get()),
		};

		zassert_ok(sys_clone(&args, &child));
		pids[i] = (pid_t)sys_process_id(child);
		/* observe the death without reaping: the zombie keeps its slot */
		zassert_ok(waitid(P_PID, (id_t)pids[i], &info, WEXITED | WNOWAIT));
	}

	errno = 0;
	zassert_equal(execve(EXEC_IMAGE, exec_argv, NULL), -1);
	zassert_equal(errno, ENOMEM);

	/* reaping the zombies unloads their images */
	for (size_t i = 0; i < ARRAY_SIZE(pids); i++) {
		zassert_equal(waitpid(pids[i], &status, 0), pids[i]);
		zassert_true(WIFEXITED(status));
		zassert_equal(WEXITSTATUS(status), 43);
	}
	zassert_is_null(llext_by_name("hello.llext"), "zombie images leaked after reap");
}

struct exec_orphan_args {
	struct k_thread *thread;
	k_thread_stack_t *stack;
};

static void exec_orphan_mid_entry(void *p1, void *p2, void *p3)
{
	struct exec_orphan_args *oa = p1;
	siginfo_t info;
	k_pid_t grand = NULL;
	struct sys_clone_args args = {
		.entry = exec_child_entry,
		.p1 = (void *)exec_exit_argv,
		.thread = oa->thread,
		.stack = oa->stack,
		.stack_size = EXEC_CHILD_STACK_SIZE,
		.prio = k_thread_priority_get(k_current_get()),
	};

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (sys_clone(&args, &grand) != 0) {
		_exit(98);
	}
	/* observe the grandchild's death without reaping it */
	if (waitid(P_PID, (id_t)sys_process_id(grand), &info, WEXITED | WNOWAIT) != 0) {
		_exit(97);
	}
	/* dying with an unreaped child: init auto-reaps the orphaned zombie,
	 * retiring its pid while its image slot is still registered
	 */
	_exit(0);
}

/* a table full of slots whose pids retired without a waitpid() is swept */
static void execve_sweep_stale_slots(void)
{
	int status = -1;
	pid_t pid;

	for (size_t i = 0; i < CONFIG_POSIX_EXEC_LLEXT_MAX; i++) {
		struct exec_orphan_args oa = {
			.thread = &exec_extra_threads[i],
			.stack = exec_extra_stacks[i],
		};
		k_pid_t mid = NULL;
		struct sys_clone_args args = {
			.entry = exec_orphan_mid_entry,
			.p1 = &oa,
			.thread = &exec_child_thread,
			.stack = exec_child_stack,
			.stack_size = EXEC_CHILD_STACK_SIZE,
			.prio = k_thread_priority_get(k_current_get()),
		};

		zassert_ok(sys_clone(&args, &mid));
		pid = (pid_t)sys_process_id(mid);
		zassert_true(pid > 0);
		zassert_equal(waitpid(pid, &status, 0), pid);
		zassert_true(WIFEXITED(status));
		zassert_equal(WEXITSTATUS(status), 0, "orphan setup failed: %d",
			      WEXITSTATUS(status));
	}

	/* every slot is stale: the next exec sweeps them and still succeeds */
	pid = exec_spawn(exec_argv);
	zassert_true(pid > 0);
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 42);
	zassert_is_null(llext_by_name("hello.llext"), "stale slots not swept");
}

ZTEST(posix_exec, test_execve)
{
	execve_return_status();
	execve_exit_reaps_image();
	execve_chain_unloads_prior();
	execve_load_errors();
	execve_args_too_big();
	execve_table_enomem();
	execve_sweep_stale_slots();
}

static void exec_setup_write(const char *path, const uint8_t *blob, size_t len)
{
	int fd;
	int ret;

	fd = open(path, O_RDWR | O_CREAT, 0644);
	zassert_true(fd >= 0, "open %s failed: %d", path, errno);
	ret = write(fd, blob, len);
	zassert_equal(ret, (int)len);
	zassert_ok(close(fd));
}

static void *exec_setup(void)
{
	zassert_ok(fs_mount(&fs_mnt));
	exec_setup_write(EXEC_IMAGE, hello_llext, sizeof(hello_llext));
	/* the same image under a second name, for the chain-exec round trip */
	exec_setup_write(EXEC_CHAIN_IMAGE, hello_llext, sizeof(hello_llext));
	exec_setup_write(EXEC_NOEXEC_IMAGE, noexec_llext, sizeof(noexec_llext));

	return NULL;
}

ZTEST_SUITE(posix_exec, NULL, exec_setup, NULL, NULL, NULL);

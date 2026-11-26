/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_func_tests.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>

#ifndef CONFIG_NATIVE_LIBC
/* without an interpreter at /bin/sh: no shell, status 127, popen() fails */
static void system_no_shell(void)
{
	int status;

	zassert_ok(unlink("/bin/sh"));

	zassert_equal(system(NULL), 0);
	status = system("exit 0");
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 127);
	errno = 0;
	zassert_is_null(popen("exit 0", "r"));
	zassert_equal(errno, ENOENT);

	zassert_ok(fs_symlink("toybox", "/bin/sh"));
}
#endif

static void system_intr_handler(int sig)
{
	ARG_UNUSED(sig);
}

static void system_sigchld_reaper(int sig)
{
	ARG_UNUSED(sig);
	while (waitpid(-1, NULL, WNOHANG) > 0) {
	}
}

#ifdef CONFIG_NATIVE_LIBC
/* the host's system() blocks the simulator whole: the command reads the caller's ignored set */
static void system_ignored_while_running(void)
{
	unsigned long long ign;
	int status;

	status = system("grep SigIgn /proc/$PPID/status | cut -f2 > " TEST_OUT);
	zassert_true(WIFEXITED(status) && (WEXITSTATUS(status) == 0), "0x%x", status);
	ign = strtoull(test_read_out(), NULL, 16);
	zassert_true((ign & (1ULL << (SIGINT - 1))) != 0,
		     "SIGINT was not ignored while the command ran");
	zassert_true((ign & (1ULL << (SIGQUIT - 1))) != 0,
		     "SIGQUIT was not ignored while the command ran");
}
#else
static volatile bool system_probe_stop;
static volatile bool system_probe_int_ignored;
static volatile bool system_probe_quit_ignored;
static K_THREAD_STACK_DEFINE(system_probe_stack, 1024);
static struct k_thread system_probe_thread;

/* a thread sampling the process-wide dispositions while a command runs */
static void system_probe(void *p1, void *p2, void *p3)
{
	struct sigaction cur;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (!system_probe_stop) {
		if ((sigaction(SIGINT, NULL, &cur) == 0) && (cur.sa_handler == SIG_IGN)) {
			system_probe_int_ignored = true;
		}
		if ((sigaction(SIGQUIT, NULL, &cur) == 0) && (cur.sa_handler == SIG_IGN)) {
			system_probe_quit_ignored = true;
		}
		k_sleep(K_MSEC(1));
	}
}

static void system_ignored_while_running(void)
{
	int status;

	system_probe_stop = false;
	system_probe_int_ignored = false;
	system_probe_quit_ignored = false;
	k_thread_create(&system_probe_thread, system_probe_stack,
			K_THREAD_STACK_SIZEOF(system_probe_stack), system_probe, NULL, NULL, NULL,
			k_thread_priority_get(k_current_get()), 0, K_NO_WAIT);
	status = system("sleep 1");
	system_probe_stop = true;
	zassert_ok(k_thread_join(&system_probe_thread, K_SECONDS(5)));
	zassert_true(WIFEXITED(status) && (WEXITSTATUS(status) == 0), "0x%x", status);
	zassert_true(system_probe_int_ignored, "SIGINT was not ignored while the command ran");
	zassert_true(system_probe_quit_ignored, "SIGQUIT was not ignored while the command ran");
}
#endif /* CONFIG_NATIVE_LIBC */

/*
 * While the command runs the caller ignores SIGINT and SIGQUIT and blocks SIGCHLD, then puts its
 * dispositions back: a reaping SIGCHLD handler cannot take the shell's status, an interrupt meant
 * for the command does not reach the caller, and the command starts with the caller's dispositions.
 */
static void system_signals(void)
{
	const struct sigaction reap = {.sa_handler = system_sigchld_reaper};
	const struct sigaction intr = {.sa_handler = system_intr_handler};
	struct sigaction old_chld;
	struct sigaction old_int;
	struct sigaction old_quit;
	struct sigaction cur;
	int status;

	zassert_ok(sigaction(SIGCHLD, &reap, &old_chld));
	status = system("exit 3");
	zassert_true(WIFEXITED(status), "0x%x", status);
	zassert_equal(WEXITSTATUS(status), 3);
	zassert_ok(sigaction(SIGCHLD, &old_chld, NULL));

	zassert_ok(sigaction(SIGINT, &intr, &old_int));
	zassert_ok(sigaction(SIGQUIT, &intr, &old_quit));
	system_ignored_while_running();
	zassert_ok(sigaction(SIGINT, NULL, &cur));
	zassert_equal(cur.sa_handler, system_intr_handler,
		      "the SIGINT disposition was not restored");
	zassert_ok(sigaction(SIGQUIT, NULL, &cur));
	zassert_equal(cur.sa_handler, system_intr_handler,
		      "the SIGQUIT disposition was not restored");
	zassert_ok(sigaction(SIGQUIT, &old_quit, NULL));
	zassert_ok(sigaction(SIGINT, &old_int, NULL));

	/* the command starts with the caller's dispositions, not the ignored ones */
	status = system("kill -INT $$");
	zassert_true(WIFSIGNALED(status) && (WTERMSIG(status) == SIGINT), "0x%x", status);
}

ZTEST(posix_shell_func, test_system)
{
	int status;

#ifndef CONFIG_NATIVE_LIBC
	system_no_shell();
#endif

	/* a shell is available */
	zassert_true(system(NULL) != 0);

	status = system("exit 0");
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 0);

	status = system("exit 3");
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 3);

	/* the command runs in the shell, with its redirections */
	status = system("echo hello from system > " TEST_OUT);
	zassert_equal(WEXITSTATUS(status), 0);
	zassert_str_equal(test_read_out(), "hello from system\n");

	/* a command the shell cannot find: its exit status, not a failure to run the shell */
	status = system("no_such_command_here");
	zassert_true(WIFEXITED(status));
	zassert_true(WEXITSTATUS(status) != 0);

	system_signals();
}

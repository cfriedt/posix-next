/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_func_tests.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/fs/fs.h>

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
}

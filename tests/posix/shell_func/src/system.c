/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_func_tests.h"

#include <stdlib.h>
#include <sys/wait.h>

ZTEST(posix_shell_func, test_system)
{
	int status;

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

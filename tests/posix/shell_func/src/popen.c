/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "shell_func_tests.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

ZTEST(posix_shell_func, test_popen)
{
	char line[64];
	FILE *f;

	/* the command's output is the stream */
	f = popen("echo popen works; echo second line", "r");
	zassert_not_null(f, "popen failed: %d", errno);
	zassert_not_null(fgets(line, sizeof(line), f));
	zassert_str_equal(line, "popen works\n");
	zassert_not_null(fgets(line, sizeof(line), f));
	zassert_str_equal(line, "second line\n");
	zassert_is_null(fgets(line, sizeof(line), f));
	zassert_equal(pclose(f), 0);

	/* the stream is the command's input */
	f = popen("cat > " TEST_OUT, "w");
	zassert_not_null(f, "popen failed: %d", errno);
	zassert_true(fputs("written through popen\n", f) >= 0);
	zassert_equal(pclose(f), 0);
	zassert_str_equal(test_read_out(), "written through popen\n");

	errno = 0;
	zassert_is_null(popen("echo", "x"));
	zassert_equal(errno, EINVAL);
}

ZTEST(posix_shell_func, test_pclose)
{
	int status;
	FILE *f;

	/* the command's exit status comes back as a wait status */
	f = popen("exit 5", "r");
	zassert_not_null(f, "popen failed: %d", errno);
	status = pclose(f);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 5);

	/* a stream popen() did not open (the host leaves this undefined) */
	IF_NOT_NATIVE_LIBC({
		f = fopen(TEST_GLOB "/a1", "r");
		zassert_not_null(f);
		errno = 0;
		zassert_equal(pclose(f), -1);
		zassert_equal(errno, EBADF);
		zassert_ok(fclose(f));
	});
}

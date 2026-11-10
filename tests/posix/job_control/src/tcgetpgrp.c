/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "job_control_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_job_control, test_tcgetpgrp)
{
	pid_t pgrp = tcgetpgrp(tty_test_fd);

	/* with no foreground process group, a positive value naming no group; else the group */
	zassert_true(pgrp > 1, "%d", (int)pgrp);
	IF_NOT_NATIVE_LIBC({
		zassert_equal(tcgetpgrp(STDOUT_FILENO), pgrp);
	});

	zassert_ok(tcsetpgrp(tty_test_fd, getpgrp()));
	zassert_equal(tcgetpgrp(tty_test_fd), getpgrp());

	errno = 0;
	zassert_equal(tcgetpgrp(-1), -1);
	zassert_equal(errno, EBADF);
}

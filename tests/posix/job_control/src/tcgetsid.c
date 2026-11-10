/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "job_control_tests.h"

#include <errno.h>
#include <termios.h>
#include <unistd.h>

ZTEST_USER(posix_job_control, test_tcgetsid)
{
	pid_t sid = tcgetsid(tty_test_fd);

	zassert_true(sid > 0, "%d", (int)sid);
	zassert_equal(sid, getsid(0));
	IF_NOT_NATIVE_LIBC({
		zassert_equal(tcgetsid(STDOUT_FILENO), sid);
	});

	errno = 0;
	zassert_equal(tcgetsid(-1), -1);
	zassert_equal(errno, EBADF);
}

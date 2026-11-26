/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "job_control_tests.h"

#include <errno.h>
#include <unistd.h>

ZTEST_USER(posix_job_control, test_tcsetpgrp)
{
	pid_t self = getpgrp();

	zassert_ok(tcsetpgrp(tty_test_fd, self));
	zassert_equal(tcgetpgrp(tty_test_fd), self);

	/* a group id must be positive (the host looks 0 up as a group and finds none) */
	errno = 0;
	zassert_equal(tcsetpgrp(tty_test_fd, 0), -1);
	zassert_true((errno == EINVAL) || (IS_ENABLED(CONFIG_NATIVE_LIBC) && (errno == ESRCH)),
		     "%d", errno);
	errno = 0;
	zassert_equal(tcsetpgrp(tty_test_fd, -1), -1);
	zassert_equal(errno, EINVAL);

	/* and name a process group in the caller's session (the host says ESRCH for none at all) */
	errno = 0;
	zassert_equal(tcsetpgrp(tty_test_fd, self + 1), -1);
	zassert_true((errno == EPERM) || (IS_ENABLED(CONFIG_NATIVE_LIBC) && (errno == ESRCH)), "%d",
		     errno);
	zassert_equal(tcgetpgrp(tty_test_fd), self);

	errno = 0;
	zassert_equal(tcsetpgrp(-1, self), -1);
	zassert_equal(errno, EBADF);
}

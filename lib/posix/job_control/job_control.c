/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_tty.h"

#include <errno.h>
#include <limits.h>
#include <termios.h>
#include <unistd.h>

#include <zephyr/sys/process.h>

pid_t tcgetpgrp(int fildes)
{
	if (posix_tty_check(fildes) < 0) {
		return -1;
	}

	/*
	 * POSIX: with no foreground process group, return a value greater
	 * than 1 that does not match any existing process group.
	 */
	return (posix_tty.fg_pgrp > 0) ? posix_tty.fg_pgrp : (pid_t)INT_MAX;
}

int tcsetpgrp(int fildes, pid_t pgid_id)
{
	if (posix_tty_check(fildes) < 0) {
		return -1;
	}

	if (pgid_id <= 0) {
		errno = EINVAL;
		return -1;
	}

	if (sys_pgrp_find((int)pgid_id) == NULL) {
		errno = EPERM;
		return -1;
	}

	posix_tty.fg_pgrp = pgid_id;

	return 0;
}

pid_t tcgetsid(int fildes)
{
	if (posix_tty_check(fildes) < 0) {
		return -1;
	}

	return getsid(0);
}

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "tty_fixture.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <zephyr/ztest.h>

#ifdef CONFIG_NATIVE_LIBC
#include <sys/ioctl.h>

static int tty_master = -1;
static int tty_slave = -1;

int posix_test_tty_open(bool controlling)
{
	tty_master = posix_openpt(O_RDWR | O_NOCTTY);
	zassert_true(tty_master >= 0, "posix_openpt() failed: %d", errno);
	zassert_ok(grantpt(tty_master));
	zassert_ok(unlockpt(tty_master));
	tty_slave = open(ptsname(tty_master), O_RDWR | O_NOCTTY);
	zassert_true(tty_slave >= 0, "open(%s) failed: %d", ptsname(tty_master), errno);

	if (controlling) {
		/* a session of our own (twister's child is no leader), then the terminal is ours */
		(void)setsid();
		zassert_ok(ioctl(tty_slave, TIOCSCTTY, 0), "TIOCSCTTY failed: %d", errno);
		/* closing the master hangs up the session: not a reason to end the test */
		(void)signal(SIGHUP, SIG_IGN);
	}

	return tty_slave;
}

void posix_test_tty_close(void)
{
	if (tty_slave >= 0) {
		(void)close(tty_slave);
		tty_slave = -1;
	}
	if (tty_master >= 0) {
		(void)close(tty_master);
		tty_master = -1;
	}
}
#else
int posix_test_tty_open(bool controlling)
{
	ARG_UNUSED(controlling);

	return STDIN_FILENO;
}

void posix_test_tty_close(void)
{
}
#endif /* CONFIG_NATIVE_LIBC */

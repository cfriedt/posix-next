/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "device_specific_tests.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* every line-control function refuses what is not a terminal the same way */
#define EXPECT_NOT_A_TTY(call_fd)                                                                  \
	do {                                                                                       \
		errno = 0;                                                                         \
		zassert_equal(call_fd(-1), -1);                                                    \
		zassert_equal(errno, EBADF);                                                       \
		errno = 0;                                                                         \
		zassert_equal(call_fd(tty_test_other_fd), -1);                                     \
		zassert_equal(errno, ENOTTY);                                                      \
	} while (0)

/*
 * The speeds are reached only through the cf*speed() functions: where they
 * live in struct termios is the implementation's business. With
 * CONFIG_POSIX_DEVICE_SPECIFIC_SHARED_IO_BAUD the terminal has one speed
 * for both directions, as the host has.
 */
#define SHARED_BAUD IS_ENABLED(CONFIG_POSIX_DEVICE_SPECIFIC_SHARED_IO_BAUD)
ZTEST_USER(posix_device_specific, test_cfgetispeed)
{
	struct termios t = {0};

	zassert_ok(cfsetispeed(&t, B9600));
	zassert_equal(cfgetispeed(&t), B9600);
	zassert_ok(cfsetispeed(&t, B115200));
	zassert_equal(cfgetispeed(&t), B115200);
}

ZTEST_USER(posix_device_specific, test_cfgetospeed)
{
	struct termios t = {0};

	zassert_ok(cfsetospeed(&t, B9600));
	zassert_equal(cfgetospeed(&t), B9600);
	zassert_ok(cfsetospeed(&t, B115200));
	zassert_equal(cfgetospeed(&t), B115200);
}

ZTEST_USER(posix_device_specific, test_cfsetispeed)
{
	struct termios t = {0};

	zassert_ok(cfsetospeed(&t, B115200));
	zassert_ok(cfsetispeed(&t, B9600));
	zassert_equal(cfgetispeed(&t), B9600);
	/* one shared rate, or the other direction's own */
	zassert_equal(cfgetospeed(&t), SHARED_BAUD ? B9600 : B115200);

	/* a zero input speed means the output speed, and keeps meaning it */
	zassert_ok(cfsetispeed(&t, 0));
	zassert_equal(cfgetispeed(&t), 0);
	zassert_equal(cfgetospeed(&t), SHARED_BAUD ? B9600 : B115200);
	zassert_ok(cfsetospeed(&t, B115200));
	zassert_equal(cfgetispeed(&t), 0);
	zassert_equal(cfgetospeed(&t), B115200);
}

ZTEST_USER(posix_device_specific, test_cfsetospeed)
{
	struct termios t = {0};

	zassert_ok(cfsetispeed(&t, B115200));
	zassert_ok(cfsetospeed(&t, B9600));
	zassert_equal(cfgetospeed(&t), B9600);
	zassert_equal(cfgetispeed(&t), SHARED_BAUD ? B9600 : B115200);
}

ZTEST_USER(posix_device_specific, test_ctermid)
{
	char buf[L_ctermid];
	const char *name = ctermid(NULL);

	zassert_not_null(name);
	zassert_true(strlen(name) < L_ctermid);
	zassert_equal(ctermid(buf), buf);
	zassert_str_equal(buf, name);
}

ZTEST_USER(posix_device_specific, test_isatty)
{
	zassert_equal(isatty(tty_test_fd), 1);
	/* the console is the standard descriptors too */
	IF_NOT_NATIVE_LIBC({
		zassert_equal(isatty(STDIN_FILENO), 1);
		zassert_equal(isatty(STDOUT_FILENO), 1);
		zassert_equal(isatty(STDERR_FILENO), 1);
	});

	errno = 0;
	zassert_equal(isatty(-1), 0);
	zassert_equal(errno, EBADF);
	errno = 0;
	zassert_equal(isatty(tty_test_other_fd), 0);
	zassert_equal(errno, ENOTTY);
}

ZTEST_USER(posix_device_specific, test_tcdrain)
{
	zassert_ok(tcdrain(tty_test_fd));
	EXPECT_NOT_A_TTY(tcdrain);
}

static int tcflow_on(int fd)
{
	return tcflow(fd, TCOON);
}

ZTEST_USER(posix_device_specific, test_tcflow)
{
	zassert_ok(tcflow(tty_test_fd, TCOOFF));
	zassert_ok(tcflow(tty_test_fd, TCOON));
	zassert_ok(tcflow(tty_test_fd, TCIOFF));
	zassert_ok(tcflow(tty_test_fd, TCION));

	errno = 0;
	zassert_equal(tcflow(tty_test_fd, 0x7f), -1);
	zassert_equal(errno, EINVAL);
	EXPECT_NOT_A_TTY(tcflow_on);
}

static int tcflush_io(int fd)
{
	return tcflush(fd, TCIOFLUSH);
}

ZTEST_USER(posix_device_specific, test_tcflush)
{
	zassert_ok(tcflush(tty_test_fd, TCIFLUSH));
	zassert_ok(tcflush(tty_test_fd, TCOFLUSH));
	zassert_ok(tcflush(tty_test_fd, TCIOFLUSH));

	errno = 0;
	zassert_equal(tcflush(tty_test_fd, 0x7f), -1);
	zassert_equal(errno, EINVAL);
	EXPECT_NOT_A_TTY(tcflush_io);
}

static int tcgetattr_into(int fd)
{
	struct termios t;

	return tcgetattr(fd, &t);
}

ZTEST_USER(posix_device_specific, test_tcgetattr)
{
	struct termios t;

	memset(&t, 0xff, sizeof(t));
	zassert_ok(tcgetattr(tty_test_fd, &t));
	/* the console starts out as a cooked, echoing terminal */
	zassert_equal(t.c_lflag & (ICANON | ECHO), ICANON | ECHO);
	zassert_equal(t.c_iflag & ICRNL, ICRNL);
	zassert_equal(t.c_cc[VMIN], 1);
	zassert_equal(t.c_cc[VTIME], 0);
	zassert_equal(cfgetispeed(&t), cfgetospeed(&t));
	EXPECT_NOT_A_TTY(tcgetattr_into);
}

static int tcsendbreak_zero(int fd)
{
	return tcsendbreak(fd, 0);
}

ZTEST_USER(posix_device_specific, test_tcsendbreak)
{
	zassert_ok(tcsendbreak(tty_test_fd, 0));
	zassert_ok(tcsendbreak(tty_test_fd, 1));
	EXPECT_NOT_A_TTY(tcsendbreak_zero);
}

static int tcsetattr_now(int fd)
{
	struct termios t = {0};

	return tcsetattr(fd, TCSANOW, &t);
}

ZTEST_USER(posix_device_specific, test_tcsetattr)
{
	struct termios saved;
	struct termios raw;
	struct termios seen;

	zassert_ok(tcgetattr(tty_test_fd, &saved));

	/* a raw mode: no line assembly, no echo, no CR translation, two bytes at a time */
	raw = saved;
	raw.c_lflag &= ~(ICANON | ECHO);
	raw.c_iflag &= ~ICRNL;
	raw.c_cc[VMIN] = 2;
	zassert_ok(tcsetattr(tty_test_fd, TCSANOW, &raw));
	zassert_ok(tcgetattr(tty_test_fd, &seen));
	zassert_equal(seen.c_lflag, raw.c_lflag);
	zassert_equal(seen.c_iflag, raw.c_iflag);
	zassert_equal(seen.c_cc[VMIN], 2);

	/* each of the optional actions takes the attributes */
	zassert_ok(tcsetattr(tty_test_fd, TCSADRAIN, &saved));
	zassert_ok(tcgetattr(tty_test_fd, &seen));
	zassert_equal(seen.c_lflag, saved.c_lflag);
	zassert_ok(tcsetattr(tty_test_fd, TCSAFLUSH, &raw));
	zassert_ok(tcgetattr(tty_test_fd, &seen));
	zassert_equal(seen.c_lflag, raw.c_lflag);

	/* an unknown action changes nothing */
	errno = 0;
	zassert_equal(tcsetattr(tty_test_fd, 0x7f, &saved), -1);
	zassert_equal(errno, EINVAL);
	zassert_ok(tcgetattr(tty_test_fd, &seen));
	zassert_equal(seen.c_lflag, raw.c_lflag);

	zassert_ok(tcsetattr(tty_test_fd, TCSANOW, &saved));
	EXPECT_NOT_A_TTY(tcsetattr_now);
}

static int ttyname_of(int fd)
{
	return (ttyname(fd) == NULL) ? -1 : 0;
}

ZTEST_USER(posix_device_specific, test_ttyname)
{
	const char *name = ttyname(tty_test_fd);

	zassert_not_null(name);
	zassert_true(strncmp(name, "/dev/", 5) == 0, "%s", name);
	/* the console is the controlling terminal, under either name */
	IF_NOT_NATIVE_LIBC({
		zassert_str_equal(name, ctermid(NULL));
		zassert_str_equal(ttyname(STDOUT_FILENO), name);
	});
	EXPECT_NOT_A_TTY(ttyname_of);
}

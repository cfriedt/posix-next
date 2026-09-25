/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_tty.h"

#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <zephyr/toolchain.h>

/*
 * The one terminal is the console. Attribute changes are kept so that they
 * round-trip, and the canonical, echo and CR-to-NL input modes reach it;
 * the console driver itself is unbuffered and synchronous, so the drain,
 * flush, flow, and break operations complete trivially.
 */

static const char posix_tty_name[] = "/dev/console";

speed_t cfgetispeed(const struct termios *termios_p)
{
	return termios_p->c_ispeed;
}

speed_t cfgetospeed(const struct termios *termios_p)
{
	return termios_p->c_ospeed;
}

int cfsetispeed(struct termios *termios_p, speed_t speed)
{
	termios_p->c_ispeed = speed;

	return 0;
}

int cfsetospeed(struct termios *termios_p, speed_t speed)
{
	termios_p->c_ospeed = speed;

	return 0;
}

char *ctermid(char *s)
{
	static char name[sizeof(posix_tty_name)];

	(void)strcpy(name, posix_tty_name);

	if (s != NULL) {
		(void)strcpy(s, posix_tty_name);
		return s;
	}

	return name;
}

int isatty(int fildes)
{
	return (posix_tty_check(fildes) == 0) ? 1 : 0;
}

int tcdrain(int fildes)
{
	return posix_tty_check(fildes);
}

int tcflow(int fildes, int action)
{
	if (posix_tty_check(fildes) < 0) {
		return -1;
	}

	if ((action != TCOOFF) && (action != TCOON) && (action != TCIOFF) && (action != TCION)) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int tcflush(int fildes, int queue_selector)
{
	if (posix_tty_check(fildes) < 0) {
		return -1;
	}

	if ((queue_selector != TCIFLUSH) && (queue_selector != TCOFLUSH) &&
	    (queue_selector != TCIOFLUSH)) {
		errno = EINVAL;
		return -1;
	}

	return 0;
}

int tcgetattr(int fildes, struct termios *termios_p)
{
	if (posix_tty_check(fildes) < 0) {
		return -1;
	}

	*termios_p = posix_tty.attrs;

	return 0;
}

int tcsendbreak(int fildes, int duration)
{
	ARG_UNUSED(duration);

	return posix_tty_check(fildes);
}

int tcsetattr(int fildes, int optional_actions, const struct termios *termios_p)
{
	if (posix_tty_check(fildes) < 0) {
		return -1;
	}

	if ((optional_actions != TCSANOW) && (optional_actions != TCSADRAIN) &&
	    (optional_actions != TCSAFLUSH)) {
		errno = EINVAL;
		return -1;
	}

	posix_tty.attrs = *termios_p;

	return posix_tty_set_mode(fildes, termios_p);
}

char *ttyname(int fildes)
{
	static char name[sizeof(posix_tty_name)];

	if (posix_tty_check(fildes) < 0) {
		return NULL;
	}

	(void)strcpy(name, posix_tty_name);

	return name;
}

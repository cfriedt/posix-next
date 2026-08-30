/*
 * Copyright (c) 2026, Friedt Professional Engineering Services, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "posix_tty.h"

#include <errno.h>
#include <stdarg.h>

#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/zvfs.h>

struct posix_tty_state posix_tty = {
	.attrs = {
		.c_iflag = ICRNL | IXON,
		.c_oflag = OPOST | ONLCR,
		.c_cflag = CREAD | CS8 | HUPCL,
		.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN,
		.c_cc = {
			[VINTR] = 0x03,  /* ^C */
			[VQUIT] = 0x1c,  /* ^\ */
			[VERASE] = 0x7f, /* DEL */
			[VKILL] = 0x15,  /* ^U */
			[VEOF] = 0x04,   /* ^D */
			[VTIME] = 0,
			[VMIN] = 1,
			[VSTART] = 0x11, /* ^Q */
			[VSTOP] = 0x13,  /* ^S */
			[VSUSP] = 0x1a,  /* ^Z */
		},
		.c_ispeed = B115200,
		.c_ospeed = B115200,
	},
};

static int posix_tty_ioctl(int fd, unsigned long request, ...)
{
	va_list args;
	int ret;

	va_start(args, request);
	ret = zvfs_ioctl(fd, request, args);
	va_end(args);

	return ret;
}

int posix_tty_check(int fd)
{
	if (posix_tty_ioctl(fd, ZFD_IOCTL_ISATTY) < 0) {
		if (errno != EBADF) {
			errno = ENOTTY;
		}
		return -1;
	}

	return 0;
}

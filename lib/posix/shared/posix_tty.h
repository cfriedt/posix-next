/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_LIB_POSIX_POSIX_TTY_H_
#define ZEPHYR_LIB_POSIX_POSIX_TTY_H_

#include <sys/types.h>
#include <termios.h>

/* the console: the one terminal the system has */
struct posix_tty_state {
	struct termios attrs;
	pid_t fg_pgrp;
};

extern struct posix_tty_state posix_tty;

/* 0 when fd refers to the terminal, else -1 with errno set */
int posix_tty_check(int fd);
int posix_tty_set_mode(int fd, const struct termios *attrs);

#endif /* ZEPHYR_LIB_POSIX_POSIX_TTY_H_ */

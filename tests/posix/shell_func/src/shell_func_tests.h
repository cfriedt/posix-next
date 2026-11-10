/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_SHELL_FUNC_TESTS_H_
#define POSIX_TESTS_SHELL_FUNC_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/*
 * /tmp exists in both worlds (the ext2 RAM disk under Zephyr, the host
 * filesystem under CONFIG_NATIVE_LIBC); the shell is toybox's at /bin/sh
 * under Zephyr and the host's on the host.
 */
#define FS_TMPDIR "/tmp"
#define TEST_ROOT FS_TMPDIR "/pshell"
#define TEST_OUT  TEST_ROOT "/out.txt"
#define TEST_GLOB TEST_ROOT "/glob"

/* the bytes TEST_OUT holds */
const char *test_read_out(void);

#endif /* POSIX_TESTS_SHELL_FUNC_TESTS_H_ */

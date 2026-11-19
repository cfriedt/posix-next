/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_JOB_CONTROL_TESTS_H_
#define POSIX_TESTS_JOB_CONTROL_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/* the controlling terminal under test */
extern int tty_test_fd;

#endif /* POSIX_TESTS_JOB_CONTROL_TESTS_H_ */

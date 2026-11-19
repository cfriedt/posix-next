/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_DEVICE_SPECIFIC_TESTS_H_
#define POSIX_TESTS_DEVICE_SPECIFIC_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/* the terminal under test, and a descriptor that is not one (the fixture's eventfd) */
extern int tty_test_fd;
extern int tty_test_other_fd;

#endif /* POSIX_TESTS_DEVICE_SPECIFIC_TESTS_H_ */

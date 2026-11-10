/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef POSIX_TESTS_USER_GROUPS_TESTS_H_
#define POSIX_TESTS_USER_GROUPS_TESTS_H_

#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"

/* an identity Zephyr does not have (a host root user may become it, so Zephyr-only) */
#define TEST_OTHER_ID 1000

#endif /* POSIX_TESTS_USER_GROUPS_TESTS_H_ */

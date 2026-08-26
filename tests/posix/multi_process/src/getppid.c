/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_getppid)
{
	zexpect_true(getppid() >= 0);
}

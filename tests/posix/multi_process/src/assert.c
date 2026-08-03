/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_multi_process, test_assert)
{
	assert(1 == 1);
}

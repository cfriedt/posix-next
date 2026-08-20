/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/ztest.h>

/*
 * exit() cannot be called from a ztest binary without terminating the test
 * runner; the exit-status observation tests live in the exit/ console-harness
 * application and, once processes exist, in child-process assertions.
 */
ZTEST_USER(posix_multi_process, test_exit)
{
	void (*fn)(int) = exit;

	zexpect_not_null(fn);
}

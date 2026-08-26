/*
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include <zephyr/ztest.h>

/*
 * _exit() cannot be called from a ztest binary without terminating the test
 * runner; the exit-status observation tests live in the exit/ console-harness
 * application and, once processes exist, in child-process assertions.
 */
ZTEST_USER(posix_multi_process, test__exit)
{
	void (*fn)(int) = _exit;

	zexpect_not_null(fn);
}

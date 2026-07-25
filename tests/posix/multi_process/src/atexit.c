/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include <zephyr/ztest.h>

static void atexit_handler(void)
{
}

ZTEST(posix_multi_process, test_atexit)
{
	zassert_ok(atexit(atexit_handler), "atexit() failed");
}

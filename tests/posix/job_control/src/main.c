/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "job_control_tests.h"

#include "../../shared/tty_fixture.h"

ZTEST_BMEM int tty_test_fd = -1;

static void *setup(void)
{
	tty_test_fd = posix_test_tty_open(true);

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	posix_test_tty_close();
}

ZTEST_SUITE(posix_job_control, NULL, setup, NULL, NULL, teardown);

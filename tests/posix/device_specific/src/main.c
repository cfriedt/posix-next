/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "device_specific_tests.h"

#include <sys/eventfd.h>
#include <unistd.h>

#include "../../shared/tty_fixture.h"

#include <zephyr/kernel.h>
#ifdef CONFIG_USERSPACE
#include <zephyr/sys/fdtable.h>
#endif

ZTEST_BMEM int tty_test_fd = -1;
ZTEST_BMEM int tty_test_other_fd = -1;

static void before(void *arg)
{
	ARG_UNUSED(arg);

	tty_test_other_fd = eventfd(0, 0);
	zassert_true(tty_test_other_fd >= 0, "eventfd() failed: %d", errno);
#ifdef CONFIG_USERSPACE
	k_object_access_all_grant(zvfs_fd_entry_get(tty_test_other_fd));
#endif
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	if (tty_test_other_fd >= 0) {
		(void)close(tty_test_other_fd);
		tty_test_other_fd = -1;
	}
}

static void *setup(void)
{
	tty_test_fd = posix_test_tty_open(false);

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	posix_test_tty_close();
}

ZTEST_SUITE(posix_device_specific, NULL, setup, before, after, teardown);

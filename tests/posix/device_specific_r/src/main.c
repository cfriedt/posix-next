/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <string.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "../../shared/linux_compat_test.h"
#ifdef CONFIG_USERSPACE
#include <zephyr/sys/fdtable.h>
#endif

#include "../../shared/tty_fixture.h"

/* the terminal under test, and an eventfd that is not one */
static ZTEST_BMEM int tty_fd = -1;
static ZTEST_BMEM int other_fd = -1;

ZTEST_USER(posix_device_specific_r, test_ttyname_r)
{
	char name[32];
	char small[4];

	memset(name, 0xff, sizeof(name));
	zassert_ok(ttyname_r(tty_fd, name, sizeof(name)));
	zassert_true(strnlen(name, sizeof(name)) < sizeof(name));
	zassert_true(strncmp(name, "/dev/", 5) == 0, "%s", name);
	memset(small, 0xff, sizeof(small));
	zassert_ok(ttyname_r(tty_fd, name, strlen(name) + 1));

	/* a buffer the name does not fit (the host is loose about one byte short) */
	zassert_equal(ttyname_r(tty_fd, small, sizeof(small)), ERANGE);
	IF_NOT_NATIVE_LIBC({
		zassert_equal(ttyname_r(tty_fd, name, strlen(name)), ERANGE);
	});

	zassert_equal(ttyname_r(-1, name, sizeof(name)), EBADF);
	zassert_equal(ttyname_r(other_fd, name, sizeof(name)), ENOTTY);
}

static void before(void *arg)
{
	ARG_UNUSED(arg);

	other_fd = eventfd(0, 0);
	zassert_true(other_fd >= 0, "eventfd() failed: %d", errno);
#ifdef CONFIG_USERSPACE
	k_object_access_all_grant(zvfs_fd_entry_get(other_fd));
#endif
}

static void after(void *arg)
{
	ARG_UNUSED(arg);

	if (other_fd >= 0) {
		(void)close(other_fd);
		other_fd = -1;
	}
}

static void *setup(void)
{
	tty_fd = posix_test_tty_open(false);

	return NULL;
}

static void teardown(void *arg)
{
	ARG_UNUSED(arg);

	posix_test_tty_close();
}

ZTEST_SUITE(posix_device_specific_r, NULL, setup, before, after, teardown);

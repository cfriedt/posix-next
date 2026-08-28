/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sched.h>

#include <zephyr/ztest.h>

ZTEST_USER(posix_sporadic_server, test_sched_getparam)
{
	struct sched_param param = {0};
	int rc = sched_getparam(0, &param);
	int err = errno;

	/* process-addressed scheduling functions are documented ENOSYS stubs */
	zassert_true((rc == -1) && (err == ENOSYS), "rc: %d errno: %d", rc, err);
}

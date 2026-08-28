/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <sched.h>

#include <zephyr/ztest.h>

#include "sporadic_server_tests.h"

ZTEST_USER(posix_sporadic_server, test_sched_setparam)
{
	struct sched_param param = sporadic_param(sched_get_priority_min(SCHED_SPORADIC));
	int rc = sched_setparam(0, &param);
	int err = errno;

	/* process-addressed scheduling functions are documented ENOSYS stubs */
	zassert_true((rc == -1) && (err == ENOSYS), "rc: %d errno: %d", rc, err);
}

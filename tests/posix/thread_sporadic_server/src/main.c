/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <limits.h>
#include <sched.h>
#include <unistd.h>

#include <zephyr/ztest.h>

BUILD_ASSERT(_POSIX_THREAD_SPORADIC_SERVER == 200809L,
	     "_POSIX_THREAD_SPORADIC_SERVER is not defined to 200809L");
BUILD_ASSERT((SCHED_SPORADIC != SCHED_OTHER) && (SCHED_SPORADIC != SCHED_FIFO) &&
		     (SCHED_SPORADIC != SCHED_RR),
	     "SCHED_SPORADIC is not distinct from the other scheduling policies");
BUILD_ASSERT(_POSIX_SS_REPL_MAX >= 4, "_POSIX_SS_REPL_MAX is below the POSIX minimum");
#ifdef SS_REPL_MAX
BUILD_ASSERT(SS_REPL_MAX >= _POSIX_SS_REPL_MAX, "SS_REPL_MAX is below the POSIX minimum");
#endif

ZTEST_SUITE(posix_thread_sporadic_server, NULL, NULL, NULL, NULL, NULL);

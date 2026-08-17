/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/ztest.h>

/*
 * Experimental CONFIG_PROCESS_VM fork over the kernel-assisted resume: the
 * child's first run returns to user mode from the parent's captured syscall
 * frame (no setjmp/longjmp), so fork() must be called from user mode - a
 * kernel-mode caller has no syscall frame and gets ENOSYS. KNOWN GAP on
 * x86 (design doc): writable app-data partitions are not yet isolated, so
 * that check is not asserted here.
 */

static ZTEST_DMEM volatile int fork_shared = 10;

ZTEST_USER(posix_fork, test_fork)
{
	volatile int on_stack = 20;

	zassert_equal(fork_shared, 10, "app-data not initialized before fork");

	pid_t pid = fork();

	zassert_true(pid >= 0, "fork failed: %d", errno);

	if (pid == 0) {
		fork_shared = 11;
		on_stack = 21;
		_exit((fork_shared == 11 && on_stack == 21) ? 7 : 1);
	}

	int status = -1;

	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 7, "child could not diverge its own copy");

	zassert_equal(on_stack, 20, "parent stack was mutated");
	zassert_equal(fork_shared, 10, "parent app-data was mutated");
}

ZTEST_SUITE(posix_fork, NULL, NULL, NULL, NULL, NULL);

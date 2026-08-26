/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <stdlib.h>
#include <sys/eventfd.h>
#include <sys/wait.h>
#include <unistd.h>

#include <zephyr/ztest.h>

/*
 * Experimental CONFIG_PROCESS_VM fork over the kernel-assisted resume: the
 * child's first run returns to user mode from the parent's captured syscall
 * frame (no setjmp/longjmp), so fork() must be called from user mode - a
 * kernel-mode caller has no syscall frame and gets ENOTSUP.
 */

static ZTEST_DMEM volatile int fork_shared = 10;
static Z_THREAD_LOCAL volatile int fork_tls = 30;

ZTEST_USER(posix_fork, test_fork)
{
	volatile int on_stack = 20;
	volatile int *tls_addr = &fork_tls;

	zassert_equal(fork_shared, 10, "app-data not initialized before fork");
	zassert_equal(fork_tls, 30, "TLS not initialized before fork");

	pid_t pid = fork();

	zassert_true(pid >= 0, "fork failed: %d", errno);

	if (pid == 0) {
		/* TLS keeps its address and contents across fork (POSIX) */
		if ((&fork_tls != tls_addr) || (fork_tls != 30)) {
			_exit(2);
		}
		fork_shared = 11;
		on_stack = 21;
		fork_tls = 31;
		_exit((fork_shared == 11 && on_stack == 21 && fork_tls == 31) ? 7 : 1);
	}

	int status = -1;

	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 7, "child could not diverge its own copy");

	zassert_equal(on_stack, 20, "parent stack was mutated");
	zassert_equal(fork_shared, 10, "parent app-data was mutated");
	zassert_equal(fork_tls, 30, "parent TLS was mutated");

	/* descriptors: the child inherits copies sharing the descriptions */
	eventfd_t val = 0;
	int efd = eventfd(0, 0);

	zassert_true(efd >= 0, "eventfd failed: %d", errno);

	pid = fork();
	zassert_true(pid >= 0, "fork failed: %d", errno);
	if (pid == 0) {
		/* the inherited descriptor reaches the same open description */
		if (eventfd_write(efd, 5) != 0) {
			_exit(3);
		}
		/* closing here drops only this process's descriptor */
		_exit((close(efd) == 0) ? 8 : 4);
	}

	status = -1;
	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 8);

	/* the parent's descriptor survives the child's close and exit */
	zassert_ok(eventfd_read(efd, &val));
	zassert_equal(val, 5);
	zassert_ok(close(efd));
}

ZTEST_SUITE(posix_fork, NULL, NULL, NULL, NULL, NULL);

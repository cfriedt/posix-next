/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <signal.h>
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

static void fork_handler(int signo)
{
	ARG_UNUSED(signo);
}

/* the child inherits every parent disposition across fork (POSIX) */
static void fork_dispositions(void)
{
	pid_t pid;
	int status = -1;
	struct sigaction oact;
	struct sigaction act = {
		.sa_handler = fork_handler,
		.sa_flags = 0,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(SIGUSR1, &act, NULL));
	act.sa_handler = SIG_IGN;
	zassert_ok(sigaction(SIGUSR2, &act, NULL));

	/* in force for the parent before the fork */
	zassert_ok(sigaction(SIGUSR1, NULL, &oact));
	zassert_equal(fork_handler, oact.sa_handler);
	zassert_ok(sigaction(SIGUSR2, NULL, &oact));
	zassert_equal(SIG_IGN, oact.sa_handler);

	pid = fork();
	zassert_true(pid >= 0, "fork failed: %d", errno);
	if (pid == 0) {
		/*
		 * Kernel-staged user memory (syscall copy-outs, the signal
		 * trampoline frame) aliases the parent's frames in the fork
		 * child, so neither sigaction() queries nor handler delivery
		 * can run here. The inherited ignore is still observable end
		 * to end: were the disposition not inherited, the default
		 * action for SIGUSR2 would end this thread before _exit(9).
		 */
		if (raise(SIGUSR2) != 0) {
			_exit(6);
		}
		_exit(9);
	}

	zassert_equal(waitpid(pid, &status, 0), pid);
	zassert_true(WIFEXITED(status));
	zassert_equal(WEXITSTATUS(status), 9,
		      "the child did not inherit the parent's dispositions (exit %d)",
		      WEXITSTATUS(status));

	act.sa_handler = SIG_DFL;
	zassert_ok(sigaction(SIGUSR1, &act, NULL));
	zassert_ok(sigaction(SIGUSR2, &act, NULL));
}

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

	fork_dispositions();
}

ZTEST_SUITE(posix_fork, NULL, NULL, NULL, NULL, NULL);

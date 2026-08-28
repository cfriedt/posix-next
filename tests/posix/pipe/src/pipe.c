/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <zephyr/ztest.h>

static ZTEST_BMEM uint8_t scratch[PIPE_BUF + 1];
static ZTEST_BMEM int shared_fd[2];
static ZTEST_BMEM volatile sig_atomic_t sigpipe_count;

static void pipe_basic(void)
{
	int fd[2];
	char buf[8];

	zassert_ok(pipe(fd));

	zassert_equal(fcntl(fd[0], F_GETFL) & O_ACCMODE, O_RDONLY);
	zassert_equal(fcntl(fd[1], F_GETFL) & O_ACCMODE, O_WRONLY);

	/* each end supports only its own direction */
	zassert_equal(write(fd[0], "x", 1), -1);
	zassert_equal(errno, EBADF);
	zassert_equal(read(fd[1], buf, sizeof(buf)), -1);
	zassert_equal(errno, EBADF);

	zassert_equal(write(fd[1], "hello", 5), 5);
	zassert_equal(read(fd[0], buf, sizeof(buf)), 5);
	zassert_mem_equal(buf, "hello", 5);

	zassert_ok(close(fd[0]));
	zassert_ok(close(fd[1]));
	zassert_equal(close(fd[0]), -1);
	zassert_equal(errno, EBADF);
}

static void pipe_eof(void)
{
	int fd[2];
	char buf[4];

	zassert_ok(pipe(fd));

	zassert_equal(write(fd[1], "abc", 3), 3);
	zassert_ok(close(fd[1]));

	/* buffered data is still readable after the write end closes */
	zassert_equal(read(fd[0], buf, 2), 2);
	zassert_mem_equal(buf, "ab", 2);
	zassert_equal(read(fd[0], buf, sizeof(buf)), 1);
	zassert_equal(buf[0], 'c');

	/* then end-of-file, persistently */
	zassert_equal(read(fd[0], buf, sizeof(buf)), 0);
	zassert_equal(read(fd[0], buf, sizeof(buf)), 0);

	zassert_ok(close(fd[0]));
}

static void pipe_nonblock(void)
{
	int fd[2];
	ssize_t drained;

	zassert_ok(pipe(fd));
	zassert_ok(fcntl(fd[0], F_SETFL, O_NONBLOCK));
	zassert_ok(fcntl(fd[1], F_SETFL, O_NONBLOCK));
	zassert_equal(fcntl(fd[0], F_GETFL) & O_NONBLOCK, O_NONBLOCK);

	/* nothing to read */
	zassert_equal(read(fd[0], scratch, sizeof(scratch)), -1);
	zassert_equal(errno, EAGAIN);

	/* a write of PIPE_BUF bytes to an empty pipe is atomic */
	zassert_equal(write(fd[1], scratch, PIPE_BUF), PIPE_BUF);

#ifndef CONFIG_TC_PROVIDES_POSIX_PIPE
	/* Zephyr's pipe capacity is exactly PIPE_BUF bytes */
	zassert_equal(write(fd[1], scratch, 1), -1);
	zassert_equal(errno, EAGAIN);

	/* atomic writes do not partially complete */
	zassert_equal(read(fd[0], scratch, 100), 100);
	zassert_equal(write(fd[1], scratch, PIPE_BUF), -1);
	zassert_equal(errno, EAGAIN);

	/* larger-than-PIPE_BUF writes may partially complete */
	zassert_equal(write(fd[1], scratch, PIPE_BUF + 1), 100);
#endif

	for (drained = 0; true; ) {
		ssize_t n = read(fd[0], scratch, sizeof(scratch));

		if (n < 0) {
			zassert_equal(errno, EAGAIN);
			break;
		}
		drained += n;
	}
	zassert_equal(drained, PIPE_BUF);

	/* O_NONBLOCK can be cleared again */
	zassert_ok(fcntl(fd[1], F_SETFL, 0));
	zassert_equal(fcntl(fd[1], F_GETFL) & O_NONBLOCK, 0);

	zassert_ok(close(fd[0]));
	zassert_ok(close(fd[1]));
}

static void pipe_sigpipe_handler(int signo)
{
	ARG_UNUSED(signo);

	++sigpipe_count;
}

static void pipe_epipe(void)
{
	int fd[2];
	sigset_t set;
	struct sigaction oact;
	struct sigaction act = {
		.sa_handler = pipe_sigpipe_handler,
	};

	zassert_ok(sigemptyset(&act.sa_mask));
	zassert_ok(sigaction(SIGPIPE, &act, &oact));
	zassert_ok(sigemptyset(&set));
	zassert_ok(sigaddset(&set, SIGPIPE));
	zassert_ok(pthread_sigmask(SIG_UNBLOCK, &set, NULL));

	zassert_ok(pipe(fd));
	zassert_ok(close(fd[0]));

	sigpipe_count = 0;
	zassert_equal(write(fd[1], "x", 1), -1);
	zassert_equal(errno, EPIPE);
	zassert_equal(sigpipe_count, 1);

	zassert_ok(close(fd[1]));
	zassert_ok(pthread_sigmask(SIG_BLOCK, &set, NULL));
	zassert_ok(sigaction(SIGPIPE, &oact, NULL));
}

static void pipe_poll(void)
{
	int fd[2];
	struct pollfd pfd[2];

	zassert_ok(pipe(fd));

	pfd[0] = (struct pollfd){.fd = fd[0], .events = POLLIN};
	pfd[1] = (struct pollfd){.fd = fd[1], .events = POLLOUT};

	/* empty pipe: writable but not readable */
	zassert_equal(poll(pfd, 2, 0), 1);
	zassert_equal(pfd[0].revents, 0);
	zassert_equal(pfd[1].revents, POLLOUT);

	zassert_equal(write(fd[1], "x", 1), 1);
	zassert_equal(poll(pfd, 2, 0), 2);
	zassert_equal(pfd[0].revents, POLLIN);
	zassert_equal(pfd[1].revents, POLLOUT);

#ifndef CONFIG_TC_PROVIDES_POSIX_PIPE
	/* full pipe: no longer writable */
	zassert_equal(write(fd[1], scratch, PIPE_BUF - 1), PIPE_BUF - 1);
	zassert_equal(poll(pfd, 2, 0), 1);
	zassert_equal(pfd[0].revents, POLLIN);
	zassert_equal(pfd[1].revents, 0);
	zassert_equal(read(fd[0], scratch, PIPE_BUF - 1), PIPE_BUF - 1);
#endif

	/* write end closed: POLLHUP, with POLLIN while data remains */
	zassert_ok(close(fd[1]));
	zassert_equal(poll(pfd, 1, 0), 1);
	zassert_equal(pfd[0].revents, POLLIN | POLLHUP);
	zassert_equal(read(fd[0], scratch, sizeof(scratch)), 1);
	zassert_equal(poll(pfd, 1, 0), 1);
	zassert_equal(pfd[0].revents, POLLHUP);
	zassert_ok(close(fd[0]));

	/* read end closed: POLLERR at the write end */
	zassert_ok(pipe(fd));
	zassert_ok(close(fd[0]));
	pfd[1] = (struct pollfd){.fd = fd[1], .events = POLLOUT};
	zassert_equal(poll(&pfd[1], 1, 0), 1);
	zassert_not_equal(pfd[1].revents & POLLERR, 0);
	zassert_ok(close(fd[1]));
}

static void *pipe_deferred_writer(void *arg)
{
	ARG_UNUSED(arg);

	k_msleep(100);
	zassert_equal(write(shared_fd[1], "z", 1), 1);

	return NULL;
}

#ifndef CONFIG_TC_PROVIDES_POSIX_PIPE
static void *pipe_deferred_reader(void *arg)
{
	ARG_UNUSED(arg);

	k_msleep(100);
	zassert_equal(read(shared_fd[0], scratch, PIPE_BUF), PIPE_BUF);

	return NULL;
}
#endif

static void pipe_blocking(void)
{
	char c;
	pthread_t th;

	zassert_ok(pipe(shared_fd));

	/* a blocked reader is woken by a writer */
	zassert_ok(pthread_create(&th, NULL, pipe_deferred_writer, NULL));
	zassert_equal(read(shared_fd[0], &c, 1), 1);
	zassert_equal(c, 'z');
	zassert_ok(pthread_join(th, NULL));

#ifndef CONFIG_TC_PROVIDES_POSIX_PIPE
	/* a blocked writer is woken by a reader draining a full pipe */
	zassert_equal(write(shared_fd[1], scratch, PIPE_BUF), PIPE_BUF);
	zassert_ok(pthread_create(&th, NULL, pipe_deferred_reader, NULL));
	zassert_equal(write(shared_fd[1], "w", 1), 1);
	zassert_ok(pthread_join(th, NULL));
	zassert_equal(read(shared_fd[0], &c, 1), 1);
	zassert_equal(c, 'w');
#endif

	zassert_ok(close(shared_fd[0]));
	zassert_ok(close(shared_fd[1]));
}

ZTEST_USER(posix_pipe, test_pipe)
{
	pipe_basic();
	pipe_eof();
	pipe_nonblock();
	pipe_epipe();
	pipe_poll();
	pipe_blocking();
}

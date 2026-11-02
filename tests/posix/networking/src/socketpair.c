/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "test_net.h"

ZTEST_USER(posix_networking, test_socketpair)
{
	int sv[2];
	char buf[8];
	ssize_t n;

	zassert_ok(socketpair(AF_UNIX, SOCK_STREAM, 0, sv));
	zassert_equal(write(sv[0], TEST_MSG, strlen(TEST_MSG)), strlen(TEST_MSG));
	n = read(sv[1], buf, sizeof(buf));
	zassert_equal(n, strlen(TEST_MSG));
	zassert_true(n < sizeof(buf));
	buf[n] = '\0';
	zassert_str_equal(buf, TEST_MSG);

	/*
	 * shutdown() ends one direction: the writer fails, the reader finds end-of-file. The
	 * failed write also raises SIGPIPE, whose default action would end the process.
	 */
	struct sigaction ign = {.sa_handler = SIG_IGN};
	struct sigaction old;

	zassert_ok(sigaction(SIGPIPE, &ign, &old));
	zassert_ok(shutdown(sv[0], SHUT_WR));
	zassert_equal(write(sv[0], "x", 1), -1);
	zassert_equal(errno, EPIPE);
	zassert_ok(sigaction(SIGPIPE, &old, NULL));
	zassert_equal(read(sv[1], buf, sizeof(buf)), 0);
	zassert_equal(write(sv[1], "y", 1), 1);
	zassert_equal(read(sv[0], buf, sizeof(buf)), 1);
	close(sv[0]);
	close(sv[1]);
}

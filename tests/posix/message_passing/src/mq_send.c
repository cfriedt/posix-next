/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_send_priority_order(void)
{
	char buf[MQ_MSG_SIZE];
	unsigned int prio;
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	zassert_ok(mq_send(mqd, "low", 4, 1));
	zassert_ok(mq_send(mqd, "high A", 7, 3));
	zassert_ok(mq_send(mqd, "mid", 4, 2));
	zassert_ok(mq_send(mqd, "high B", 7, 3));

	/* highest priority first, FIFO within a priority */
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), &prio), 7);
	zassert_equal(prio, 3);
	zassert_mem_equal(buf, "high A", 7);

	zassert_equal(mq_receive(mqd, buf, sizeof(buf), &prio), 7);
	zassert_equal(prio, 3);
	zassert_mem_equal(buf, "high B", 7);

	zassert_equal(mq_receive(mqd, buf, sizeof(buf), &prio), 4);
	zassert_equal(prio, 2);
	zassert_mem_equal(buf, "mid", 4);

	zassert_equal(mq_receive(mqd, buf, sizeof(buf), &prio), 4);
	zassert_equal(prio, 1);
	zassert_mem_equal(buf, "low", 4);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_send_variable_length(void)
{
	char buf[MQ_MSG_SIZE];
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* a zero-length message is a message */
	zassert_ok(mq_send(mqd, "", 0, 0));
	zassert_ok(mq_send(mqd, "1234567", 8, 0));

	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), 0);
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), 8);
	zassert_mem_equal(buf, "1234567", 8);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_send_nonblock(void)
{
	mqd_t mqd = mq_test_open(O_NONBLOCK);

	zassert_not_equal(mqd, (mqd_t)-1);

	mq_test_fill(mqd, 0);

	/* a full queue on a non-blocking descriptor does not wait */
	zassert_equal(mq_send(mqd, "overflow", 9, 0), -1);
	zassert_equal(errno, EAGAIN);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_send_errors(void)
{
	char big[MQ_MSG_SIZE + 1] = {0};
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* a message larger than the queue's message size */
	zassert_equal(mq_send(mqd, big, sizeof(big), 0), -1);
	zassert_equal(errno, EMSGSIZE);

	/* a priority the implementation does not support */
	zassert_equal(mq_send(mqd, "prio", 5, MQ_PRIO_MAX), -1);
	zassert_equal(errno, EINVAL);

	/* a descriptor that is not open */
	zassert_equal(mq_send((mqd_t)-1, "bad", 4, 0), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

ZTEST_USER(posix_message_passing, test_mq_send)
{
	mq_send_priority_order();
	mq_test_section_reset();
	mq_send_variable_length();
	mq_test_section_reset();
	mq_send_nonblock();
	mq_test_section_reset();
	mq_send_errors();
	mq_test_section_reset();
}

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_receive_reports_length_and_priority(void)
{
	char buf[MQ_MSG_SIZE];
	unsigned int prio = 0xffffffff;
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	zassert_ok(mq_send(mqd, "sized", 6, 5));

	/* the actual sent length is returned, not the queue's message size */
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), &prio), 6);
	zassert_equal(prio, 5);
	zassert_mem_equal(buf, "sized", 6);

	/* a NULL priority pointer is accepted */
	zassert_ok(mq_send(mqd, "again", 6, 2));
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), 6);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_receive_nonblock(void)
{
	char buf[MQ_MSG_SIZE];
	mqd_t mqd = mq_test_open(O_NONBLOCK);

	zassert_not_equal(mqd, (mqd_t)-1);

	/* an empty queue on a non-blocking descriptor does not wait */
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), -1);
	zassert_equal(errno, EAGAIN);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_receive_errors(void)
{
	char buf[MQ_MSG_SIZE];
	char small[MQ_MSG_SIZE - 1];
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);
	zassert_ok(mq_send(mqd, "msg", 4, 0));

	/* the receive buffer must be able to hold a maximum-size message */
	zassert_equal(mq_receive(mqd, small, sizeof(small), NULL), -1);
	zassert_equal(errno, EMSGSIZE);

	/* the message is still queued */
	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), 4);

	/* a descriptor that is not open */
	zassert_equal(mq_receive((mqd_t)-1, buf, sizeof(buf), NULL), -1);
	zassert_equal(errno, EBADF);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

ZTEST_USER(posix_message_passing, test_mq_receive)
{
	mq_receive_reports_length_and_priority();
	mq_test_section_reset();
	mq_receive_nonblock();
	mq_test_section_reset();
	mq_receive_errors();
	mq_test_section_reset();
}

/*
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "message_passing_tests.h"

static void mq_getattr_reports_geometry(void)
{
	struct mq_attr attr;
	char buf[MQ_MSG_SIZE];
	mqd_t mqd = mq_test_open(0);

	zassert_not_equal(mqd, (mqd_t)-1);

	zassert_ok(mq_getattr(mqd, &attr));
	zassert_equal(attr.mq_msgsize, MQ_MSG_SIZE);
	zassert_equal(attr.mq_maxmsg, MQ_MAX_MSGS);
	zassert_equal(attr.mq_curmsgs, 0);
	zassert_equal(attr.mq_flags, 0);

	/* mq_curmsgs tracks the queue depth */
	zassert_ok(mq_send(mqd, "one", 4, 0));
	zassert_ok(mq_getattr(mqd, &attr));
	zassert_equal(attr.mq_curmsgs, 1);

	zassert_equal(mq_receive(mqd, buf, sizeof(buf), NULL), 4);
	zassert_ok(mq_getattr(mqd, &attr));
	zassert_equal(attr.mq_curmsgs, 0);

	zassert_ok(mq_close(mqd));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_getattr_flags_are_per_descriptor(void)
{
	struct mq_attr attr;
	mqd_t blocking = mq_test_open(0);
	mqd_t nonblock = mq_open(MQ_NAME, O_RDWR | O_NONBLOCK);

	zassert_not_equal(blocking, (mqd_t)-1);
	zassert_not_equal(nonblock, (mqd_t)-1);

	zassert_ok(mq_getattr(nonblock, &attr));
	zassert_equal(attr.mq_flags, O_NONBLOCK);

	/* the other descriptor for the same queue is unaffected */
	zassert_ok(mq_getattr(blocking, &attr));
	zassert_equal(attr.mq_flags, 0);

	zassert_ok(mq_close(nonblock));
	zassert_ok(mq_close(blocking));
	zassert_ok(mq_unlink(MQ_NAME));
}

static void mq_getattr_errors(void)
{
	struct mq_attr attr;

	zassert_equal(mq_getattr((mqd_t)-1, &attr), -1);
	zassert_equal(errno, EBADF);
}

ZTEST_USER(posix_message_passing, test_mq_getattr)
{
	mq_getattr_reports_geometry();
	mq_test_section_reset();
	mq_getattr_flags_are_per_descriptor();
	mq_test_section_reset();
	mq_getattr_errors();
	mq_test_section_reset();
}
